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

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <ctype.h>

#include "options.h"
#include "global.h"

#include "utils/text.h"
#include "utils/string.h"

/* lowercase a char */
char lowerCase(char a)
{
    if (a >= 'A' && a <= 'Z')
        return a - 'A'+ 'a' ;
    else
        return a;
}

/* test if b starts with a
 */
bool startWith (const char *a, const char *b)
{
    for (uint i = 0; a[i] != 0; i++)
        if (a[i] != b[i])
            return false;
    return true;
}

/* test if b is forbidden by pattern a */
bool robotsMatch (const char *a, const char *b)
{
    uint i = 0;
    uint j = 0;
    while (a[i] != 0)
        if (a[i] == '*')
        {
            i++;
            const char *tmp = strchr(b + j, a[i]);
            if (tmp == NULL)
                return false;
            j = tmp - b;
        }
        else
        {
            if (a[i] != b[j])
                return false;
            i++;
            j++;
        }
    return true;
}

/* test if b starts with a ignoring case
 */
bool startWithIgnoreCase (const char *amin, const char *b)
{
    for (uint i = 0; amin[i] != 0; i++)
        if (amin[i] != (b[i] | 0x20))
            return false;
    return true;
}

/* test if b end with a
 */
bool endWith (const char *a, const char *b)
{
    uint la = strlen(a);
    uint lb = strlen(b);
    return (la <= lb) && !strcmp(a, b + lb - la);
}

/* test if b end with a ignoring case
 * a can use min char, '.' (a[i] = a[i] | 32)
 */
bool endWithIgnoreCase (const char *amin, const char *b, uint lb)
{
    uint la = strlen(amin);
    if (la <= lb)
    {
        uint diff = lb - la;
        for (uint i = 0; i < la; i++)
            if (amin[i] != (b[diff + i] | 0x20))
                return false;
        return true;
    }
    return false;
}

/* test if b contains a */
bool caseContain (const char *a, const char *b)
{
    size_t la = strlen(a);
    for (int i = strlen(b) - la; i >= 0; i--)
        if (!strncasecmp(a, b + i, la))
            return true;
    return false;
}

/* create a copy of a string
 */
char *newString (const char *arg)
{
    char *res = new char[strlen(arg) + 1];
    strcpy(res, arg);
    return res;
}

/* Read a whole file
 */
char *readfile (int fds)
{
    ssize_t pos = 0;
    ssize_t size = 512;
    int cont = 1;
    char buf[500];
    ssize_t nbRead;
    char *res = new char[size];
    while(cont == 1)
    {
        switch (nbRead = read(fds, &buf, 500))
        {
        case 0 :
            cont = 0;
            break;
        case -1 :
            if (errno != EINTR && errno != EIO)
                cont = -1;
            break;
        default :
            if (pos + nbRead >= size)
            {
                size *= 2;
                char *tmp = new char[size];
                memcpy(tmp, res, pos);
                delete res;
                res = tmp;
            }
            memcpy(res + pos, buf, nbRead);
            pos += nbRead;
            break;
        }
    }
    res[pos] = 0;
    return res;
}

/* find the next token in the robots.txt, or in config file
 * must delete comments
 * no allocation (cf strtok); content is changed
 */
char *nextToken(char **posParse, char c)
{
    // go to the beginning of next word
    bool cont = 1;
    while (cont)
        if (**posParse == c || **posParse == ' ' || **posParse == '\t' || **posParse == '\r' || **posParse == '\n')
            (*posParse)++;
        else if (**posParse == '#')
        {
            *posParse = strchr(*posParse, '\n');
            if (*posParse == NULL)
                return NULL;
            else
                (*posParse)++;
        }
        else
            cont=0;
    // find the end of this word
    char *deb = *posParse;
    if (**posParse == '\"')
    {
        deb++;
        (*posParse)++;
        while (**posParse != 0 && **posParse != '\"')
            (*posParse)++;
    }
    else
    {
        while(**posParse != 0 && **posParse != c && **posParse != ' ' && **posParse != '\t' && **posParse != '\r' && **posParse != '\n')
            (*posParse)++;
        if (*posParse == deb)
            return NULL; // EOF
    }
    if (**posParse != 0)
    {
        **posParse = 0;
        (*posParse)++;
    }
    return deb;
}

/* does this char * match privilegedExt */
bool matchPrivExt (const char *file)
{
    if (!global::specificSearch)
        return false;
    for (int len = strlen(file), i = 0; global::privilegedExts[i] != NULL; i++)
        if (endWithIgnoreCase(global::privilegedExts[i], file, len))
            return true;
    return false;
}

/* does this char * match contentType */
int matchContentType (const char *ct)
{
    if (!global::specificSearch)
        return false;
    for (int i = 0; global::contentTypes[i] != NULL; i++)
        if (startWithIgnoreCase(global::contentTypes[i], ct))
            return i;
    return -1;
}

// end of text.cc
