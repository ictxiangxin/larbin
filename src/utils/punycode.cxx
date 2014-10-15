/*
 *   Larbin - is a web crawler
 *   Copyright (C) 2013  ictxiangxin
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdint.h>

#include "utils/text.h"

#define BASE         36
#define TMIN         1
#define TMAX         26
#define SKEW         38
#define DAMP         700
#define INITIAL_N    128
#define INITIAL_BIAS 72

static uint32_t adapt_bias(uint32_t delta, uint32_t n_points, bool is_first)
{
    uint32_t k;
    delta /= is_first ? DAMP : 2;
    delta += delta / n_points;
    // while delta > 455: delta /= 35
    for (k = 0; delta > ((BASE - TMIN) * TMAX) / 2; k += BASE)
        delta /= (BASE - TMIN);
    return k + (((BASE - TMIN + 1) * delta) / (delta + SKEW));
}

static char encode_digit(uint32_t c)
{
    assert(c >= 0 && c <= BASE - TMIN);
    if (c > 25)
        return c + 22; // '0'..'9'
    else
        return c + 'a'; // 'a'..'z'
}

static uint32_t encode_var_int(const uint32_t bias, const uint32_t delta, unsigned char *const dst, uint32_t dstlen)
{
    uint32_t i, k, q, t;
    i = 0;
    k = BASE;
    q = delta;
    while (i < dstlen)
    {
        if (k <= bias)
            t = TMIN;
        else if (k >= bias + TMAX)
            t = TMAX;
        else
            t = k - bias;
        if (q < t)
            break;
        dst[i++] = encode_digit(t + (q - t) % (BASE - t));
        q = (q - t) / (BASE - t);
        k += BASE;
    }
    if (i < dstlen)
        dst[i++] = encode_digit(q);

    return i;
}

static uint16_t *utf8_to_unicode(const char * src, uint32_t *len)
{
    uint32_t srclen = strlen(src);
    uint16_t *unicode = new uint16_t[srclen + 1];
    uint32_t di = 0;
    uint32_t si = 0;
    for (; si < srclen - 2;)
        if ((src[si] & 0xf0) == 0xe0)
        {
            unicode[di] = src[si++] & 0x0f;
            unicode[di] = (unicode[di] << 6) + (src[si++] & 0x3f);
            unicode[di] = (unicode[di] << 6) + (src[si++] & 0x3f);
            di++;
        }
        else
            unicode[di++] = src[si++];
    if (si < srclen)
    {
        unicode[di++] = src[si++];
        unicode[di++] = src[si++];
    }
    unicode[di] = 0;
    *len = di;
    return unicode;
}

static char *punycode_encode(const char *src_str, uint32_t *dst_len)
{
    uint32_t b, h;
    uint32_t delta, bias;
    uint32_t m, n;
    uint32_t si = 0;
    uint32_t di = 0;
    uint32_t srclen;
    uint16_t *src = utf8_to_unicode(src_str, &srclen);
    uint32_t dstlen = 8 * srclen + 8;
    char *dst_str = new char[dstlen];
    strcpy(dst_str, "xn--");
    unsigned char *dst = (unsigned char *)(dst_str + 4);
    for (; si < srclen && di < dstlen - 4; si++)
        if (src[si] < 0x80)
            dst[di++] = src[si];
    b = h = di;
    if (di != 0)
        dst[di++] = '-';
    n = INITIAL_N;
    bias = INITIAL_BIAS;
    delta = 0;
    for (; h < srclen && di < dstlen - 4; n++, delta++)
    {
        for (m = UINT32_MAX, si = 0; si < srclen; si++)
            if (src[si] >= n && src[si] < m)
                m = src[si];
        if ((m - n) > (UINT32_MAX - delta) / (h + 1))
            goto error;
        delta += (m - n) * (h + 1);
        n = m;
        for (si = 0; si < srclen; si++)
            if (src[si] < n)
            {
                if (++delta == 0)
                    goto error;
            }
            else if (src[si] == n)
            {
                di += encode_var_int(bias, delta, &dst[di], dstlen - di);
                bias = adapt_bias(delta, h + 1, h == b);
                delta = 0;
                h++;
            }
    }
    dst[di] ='\0';
    *dst_len = di + 4;
    goto finally;
error:
    delete [] dst_str;
    *dst_len = 0;
    dst_str = NULL;
finally:
    delete [] src;
    return dst_str;
}

static bool have_unicode(const char *str, uint32_t len)
{
    uint32_t i;
    for(i = 0; !(str[i] & 0x80) && i < len; i++);
    if (i == len)
        return false;
    return true;
}

char *punycode_host(const char *host)
{
    // fast check
    uint32_t hostlen = strlen(host);
    if (!have_unicode(host, hostlen))
        return (char*)host;
    char *pchost = new char[8 * hostlen];
    uint32_t pchost_i = 0;
    uint32_t start = 0;
    uint32_t size = 0;
    uint32_t end = 0;
    char flag = '#';
    for (uint32_t i = 0; i < hostlen + 1; i++)
    {
        if (host[i] == '.')
        {
            flag = '.';
            size = 1;
        }
        if (host[i] == '\0')
        {
            flag = '\0';
            size = 1;
        }
        if (host[i] == (char)0xe3 && host[i + 1] == (char)0x80 && host[i + 2] == (char)0x82)
        {
            flag = '.';
            size = 3;
        }
        if (flag != '#')
        {
            end = i;
            uint32_t temp_len = end - start + 1;
            char *substr = new char[temp_len + 1];
            strncpy(substr, host + start, temp_len);
            substr[temp_len - 1] = '\0';
            if (have_unicode(substr, temp_len))
            {
                uint32_t temp_encode_len;
                char *temp_encode = punycode_encode(substr, &temp_encode_len);
                if (temp_encode)
                {
                    strcpy(pchost + pchost_i, temp_encode);
                    pchost_i += temp_encode_len;
                    delete [] temp_encode;
                    pchost[pchost_i++] = flag;
                }
            }
            else
            {
                strncpy(pchost + pchost_i, substr, temp_len);
                pchost_i += temp_len - 1;
                pchost[pchost_i++] = flag;
            }
            delete [] substr;
            start = end + size;
            flag = '#';
        }
    }
    return pchost;
}
