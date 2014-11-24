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

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/url.h"
#include "utils/text.h"
#include "utils/connection.h"
#include "utils/punycode.h"
#include "utils/debug.h"

#define initCookie() cookie = NULL

/* small functions used later */
static uint siteHashCode (char *host)
{
    uint h = 0;
    for (uint i = 0; host[i] != 0; i++)
        h = 37 * h + host[i];
    return h % namedSiteListSize;
}

/*
 * return the int with correspond to a char
 * -1 if not an hexa char
 */
static int hexToInt (char c)
{
    if (c >= '0' && c <= '9')
        return (c - '0');
    else if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    else if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    else
        return -1;
}

static void setHexStr (char *location, char c)
{
    char lc = c & 0xf;
    char hc = (c >> 0x4) & 0xf;
    if(hc > 9)
        location[0] = hc - 0xa + 'a';
    else
        location[0] = hc + '0';
    if(lc > 9)
        location[1] = lc - 0xa + 'a';
    else
        location[1] = lc + '0';
}

/*
 * normalize a file name : also called by robots.txt parser
 * return true if it is ok, false otherwise (cgi-bin)
 */
bool fileNormalize (char *file)
{
    int i = 0;
    while (file[i] != 0 && file[i] != '#')
    {
        if (file[i] == '/')
        {
            if (file[i + 1] == '.' && file[i + 2] == '/')
            {
                // suppress /./
                uint j = i + 3;
                for (; file[j] != 0; j++)
                    file[j - 2] = file[j];
                file[j - 2] = 0;
            }
            else if (file[i + 1] == '/')
            {
                // replace // by /
                uint j = i + 2;
                for (; file[j] != 0; j++)
                    file[j - 1] = file[j];
                file[j - 1] = 0;
            }
            else if (file[i + 1] == '.' && file[i + 2] == '.' && file[i + 3] == '/')
            {
                // suppress /../
                if (i == 0) // the file name starts with /../ : error
                    return false;
                else
                {
                    uint j = i + 4;
                    uint dec;
                    i--;
                    while (file[i] != '/')
                        i--;
                    dec = i + 1 - j; // dec < 0
                    for (; file[j] != 0; j++)
                        file[j + dec] = file[j];
                    file[j + dec] = 0;
                }
            }
            else if (file[i + 1] == '.' && file[i + 2] == 0)
            {
                // suppress /.
                file[i + 1] = 0;
                return true;
            }
            else if (file[i + 1] == '.' && file[i + 2] == '.' && file[i + 3] == 0)
            {
                // suppress /..
                if (i == 0) // the file name starts with /.. : error
                    return false;
                else
                {
                    i--;
                    while (file[i] != '/')
                        i--;
                    file[i + 1] = 0;
                    return true;
                }
            }
            else // nothing special, go forward
                i++;
        }
        else if (file[i] == '%')
        {
            int v1 = hexToInt(file[i + 1]);
            int v2 = hexToInt(file[i + 2]);
            if (v1 < 0 || v2 < 0)
                return false;
            char c = 16 * v1 + v2;
            if (isgraph(c))
            {
                file[i] = c;
                int j = i + 3;
                for (; file[j] != 0; j++)
                    file[j - 2] = file[j];
                file[j - 2] = 0;
                i++;
            }
            else if (c == ' ' || c == '/') // keep it with the % notation
                i += 3;
            else // bad url
                return false;
        }
        else// nothing special, go forward
            i++;
    }
    file[i] = 0;
    return true;
}

// definition of methods of class url

// Constructor : Parses an url
url::url (char *u, int depth, url *base)
{
    newUrl();
    this->depth = depth;
    host = NULL;
    port = 80;
    file = NULL;
    initCookie();
#ifdef URL_TAGS
    tag = 0;
#endif // URL_TAGS
    if (startWith((char*)"http://", u))
    {
        // absolute url
        parse (u + 7);
        // normalize file name
        if (file != NULL && !normalize())
        {
            delete [] file;
            file = NULL;
            delete [] host;
            host = NULL;
        }
    }
    else if (base != NULL)
    {
        if (startWith((char*)"http:", u))
            parseWithBase(u + 5, base);
        else if (isProtocol(u))
            ;// Unknown protocol (mailto, ftp, news, file, gopher...)
        else
            parseWithBase(u, base);
    }
    punycode = NULL;
}

/* constructor used by input */
url::url (char *line,  int depth)
{
    newUrl();
    this->depth = depth;
    host = NULL;
    port = 80;
    file = NULL;
    initCookie();
    uint i = 0;
#ifdef URL_TAGS
    tag = 0;
    for (; line[i] >= '0' && line[i] <= '9'; i++)
        tag = 10 * tag + line[i] - '0';
    i++;
#endif // URL_TAGS
    if (startWith((char*)"http://", line + i))
    {
        parse(line + i + 7);
        // normalize file name
        if (file != NULL && !normalize())
        {
            delete [] file;
            file = NULL;
            delete [] host;
            host = NULL;
        }
    }
    punycode = NULL;
}

// Constructor : read the url from a file (cf serialize)
url::url (char *line)
{
    newUrl();
    uint i = 0;
    // Read depth
    depth = 0;
    for (; line[i] >= '0' && line[i] <= '9'; i++)
        depth = 10 * depth + line[i] - '0';
#ifdef URL_TAGS
    // read tag
    tag = 0;
    i++;
    while (; line[i] >= '0' && line[i] <= '9'; i++)
        tag = 10*tag + line[i] - '0';
#endif // URL_TAGS
    int deb = ++i;
    // Read host
    while (line[i] != ':')
        i++;
    line[i] = 0;
    host = newString(line + deb);
    i++;
    // Read port
    port = 0;
    for (; line[i] >= '0' && line[i] <= '9'; i++)
        port = 10*port + line[i] - '0';
    // Read file name
    if(!global::useCookies)
        file = newString(line + i);
    else
    {
        char *cpos = strchr(line + i, ' ');
        if (cpos == NULL)
            cookie = NULL;
        else
        {
            *cpos = 0;
            // read cookies
            cookie = new char[maxCookieSize];
            strcpy(cookie, cpos + 1);
        }
        // Read file name
        file = newString(line + i);
    }
    punycode = NULL;
}

/* constructor used by giveBase */
url::url (char *host, uint port, char *file)
{
    newUrl();
    initCookie();
    this->host = host;
    this->port = port;
    this->file = file;
    this->punycode = NULL;
}

/* Destructor */
url::~url ()
{
    delUrl();
    delete [] host;
    delete [] file;
    if (punycode && punycode != host)
        delete [] punycode;
    if(global::useCookies)
        delete [] cookie;
}

/* Is it a valid url ? */
bool url::isValid ()
{
    if (host == NULL)
        return false;
    int lh = strlen(host);
    return (file != NULL) && (lh < maxSiteSize) && (lh + strlen(file) + 18 < maxUrlSize);
}

/* print an URL */
void url::print ()
{
    printf("http://%s:%u%s\n", host, port, file);
}

/* get punycode host */
char *url::getPunycode()
{
    if (!global::punycode)
        return host;
    if (punycode == NULL)
        punycode = punycode_host(host);
    return punycode;
}

/* Set depth to max if necessary
 * try to find the ip addr
 * answer false if forbidden by robots.txt, true otherwise */
bool url::initOK (url *from)
{
    if (strcmp(from->getHost(), host))
    {
        // different site
        if(global::lockSite)
        {
            errno = outSite;
            return false;
        }
        if(global::depthBySite)
            depth = global::depthInSite;
    }
    else
    {
        // same site
        if(global::useCookies)
            if (from->cookie != NULL)
            {
                cookie = new char[maxCookieSize];
                strcpy(cookie, from->cookie);
            }
    }
    if (depth < 0)
    {
        errno = tooDeep;
        return false;
    }
    NamedSite *ns = global::namedSiteList + (hostHashCode());
    if (!strcmp(ns->name, host) && ns->port == port)
    {
        switch (ns->dnsState)
        {
            case errorDns:
                errno = fastNoDns;
                return false;
            case noConnDns:
                errno = fastNoConn;
                return false;
            case doneDns:
                if (!ns->testRobots(file))
                {
                    errno = fastRobots;
                    return false;
                }
        }
    }
    return true;
}

/* return the base of the url */
url *url::giveBase ()
{
    int i = strlen(file);
    assert (file[0] == '/');
    while (file[i] != '/')
        i--;
    char *newFile = new char[i + 2];
    memcpy(newFile, file, i + 1);
    newFile[i + 1] = 0;
    return new url(newString(host), port, newFile);
}

/** return a char * representation of the url
 * give means that you have to delete the string yourself
 */
char *url::giveUrl ()
{
    char *tmp;
    int i = strlen(file);
    uint j = strlen(host);

    tmp = new char[18 + i + j];  // 7 + j + 1 + 9 + i + 1
    // http://(host):(port)(file)\0
    strcpy(tmp, "http://");
    strcpy (tmp + 7, host);
    j += 7;
    if (port != 80)
        j += sprintf(tmp + j, ":%u", port);
    // Copy file name
    for (; i >= 0; i--)
        tmp [j + i] = file[i];
    return tmp;
}

/*
 * write the url in a buffer
 * buf must be at least of size maxUrlSize
 * returns the size of what has been written (not including '\0')
 */
int url::writeUrl (char *buf)
{
    if (port == 80)
        return sprintf(buf, "http://%s%s", host, file);
    else
        return sprintf(buf, "http://%s:%u%s", host, port, file);
}

/* serialize the url for the Persistent Fifo */
char *url::serialize ()
{
    // this buffer is protected by the lock of PersFifo
    static char statstr[maxUrlSize + 40 + maxCookieSize];
    int pos = sprintf(statstr, "%u ", depth);
#ifdef URL_TAGS
    pos += sprintf(statstr+pos, "%u ", tag);
#endif // URL_TAGS
    pos += sprintf(statstr+pos, "%s:%u%s", host, port, file);
    if(global::useCookies)
        if (cookie != NULL)
            pos += sprintf(statstr+pos, " %s", cookie);
    statstr[pos] = '\n';
    statstr[pos+1] = 0;
    return statstr;
}

/* very thread unsafe serialisation in a static buffer */
char *url::getUrl()
{
    static char statstr[maxUrlSize + 40];
    sprintf(statstr, "http://%s:%u%s", host, port, file);
    return statstr;
}

/* return a hashcode for the host of this url */
uint url::hostHashCode ()
{
    return siteHashCode (host);
}

/* return a hashcode for this url */
uint url::hashCode ()
{
    uint h = port;
    for (uint i = 0; host[i] != 0; i++)
        h = 31 * h + host[i];
    for (uint i = 0; file[i] != 0; i++)
        h = 31 * h + file[i];
    return h % hashSize;
}

/* parses a url :
 * at the end, arg must have its initial state,
 * http:// has allready been suppressed
 */
void url::parse (char *arg)
{
    uint deb = 0, fin = deb;
    // Find the end of host name (put it into lowerCase)
    while (arg[fin] != '/' && arg[fin] != ':' && arg[fin] != 0)
        fin++;
    if (fin == 0)
        return;

    // get host name
    host = new char[fin + 1];
    for (uint i = 0; i < fin; i++)
        host[i] = lowerCase(arg[i]);
    host[fin] = 0;

    // get port number
    if (arg[fin] == ':')
    {
        port = 0;
        fin++;
        while (arg[fin] >= '0' && arg[fin] <= '9')
        {
            port = port * 10 + arg[fin]-'0';
            fin++;
        }
    }

    // get file name
    if (arg[fin] != '/')
    {
        // www.inria.fr => add the final /
        if(file != NULL)
            delete [] file;
        file = newString((char*)"/");
    }
    else
    {
        if(file != NULL)
            delete [] file;
        file = newString(arg + fin);
    }
}

/** parse a file with base
 */
void url::parseWithBase (char *u, url *base)
{
    // cat filebase and file
    if (u[0] == '/')
    {
        if(file != NULL)
            delete [] file;
        file = newString(u);
    }
    else
    {
        uint lenb = strlen(base->file);
        char *tmp = new char[lenb + strlen(u) + 1];
        memcpy(tmp, base->file, lenb);
        strcpy(tmp + lenb, u);
        file = tmp;
    }
    if (!normalize())
    {
        delete [] file;
        file = NULL;
        return;
    }
    if(host != NULL)
        delete [] host;
    host = newString(base->host);
    port = base->port;
}

/*
 * normalize file name
 * return true if it is ok, false otherwise (cgi-bin)
 */
bool url::normalize ()
{
    if(fileNormalize(file))
    {
        bool normal = true;
        for (uint i = 0; file[i] != '\0' && normal; i++)
            if(file[i] < 0)
                normal = false;
        if(normal)
            return true;
        char *extFile = new char[strlen(file) * 3 + 1];
        uint i = 0;
        uint j = 0;
        while(file[i] != '\0')
            if(file[i] < 0)
            {
                extFile[j++] = '%';
                setHexStr(extFile + j, file[i++]);
                j += 2;
            }
            else
                extFile[j++] = file[i++];
        extFile[j] = '\0';
        delete [] file;
        file = extFile;
        return true;
    }
    return false;
}

/* Does this url starts with a protocol name */
bool url::isProtocol (char *s)
{
    uint i = 0;
    while (isalnum(s[i]))
        i++;
    return s[i] == ':';
}

#define addToCookie(s) \
    do { \
        len = strlen(cookie); \
        strncpy(cookie + len, s, maxCookieSize - len); \
        cookie[maxCookieSize - 1] = 0; \
    } while(0)

/* see if a header contain a new cookie */

void url::addCookie(char *header)
{
    if (startWithIgnoreCase("set-cookie: ", header))
    {
        char *pos = strchr(header + 12, ';');
        if (pos != NULL)
        {
            int len;
            if (cookie == NULL)
            {
                cookie = new char[maxCookieSize];
                cookie[0] = 0;
            }
            else
                addToCookie("; ");
            *pos = 0;
            addToCookie(header + 12);
            *pos = ';';
        }
    }
}
