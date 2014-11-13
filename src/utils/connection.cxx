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

// various functions for writing

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ctype.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>

#include "options.h"

/*
 * make write until everything is written
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrire (int fd, const char *buf)
{
    int pos = 0;
    int count = strlen(buf);
    while (pos < count)
    {
        int i = write(fd, buf + pos, count - pos);
        if (i == -1)
        {
            if (errno != EINTR)
                pos = count + 1;
        }
        else
            pos += i;
    }
    return pos != count;
}

/*
 * make write until everything is written
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrireBuff (int fd, const char *buf, int count)
{
    int pos = 0;
    while (pos < count)
    {
        int i = write(fd, buf + pos, count - pos);
        if (i == -1)
        {
            switch (errno)
            {
            case EINTR:
                break;
            default:
                pos = count + 1;
                perror("Problem in ecrireBuff");
                break;
            }
        }
        else
            pos += i;
    }
    return pos != count;
}



/*
 * Write an int on a fds
 * (uses ecrire)
 */
int ecrireInt (int fd, int i)
{
    char buf[32];
    sprintf(buf, "%d", i);
    return ecrire(fd, buf);
}

int ecrireInt2 (int fd, int i)
{
    char buf[32];
    sprintf(buf, "%d%c", i / 10, i % 10 + '0');
    return ecrire(fd, buf);
}

int ecrireInti (int fd, int i, const char *f)
{
    char buf[128];
    sprintf(buf, f, i);
    return ecrire(fd, buf);
}

int ecrireIntl (int fd, long i, const char *f)
{
    char buf[128];
    sprintf(buf, f, i);
    return ecrire(fd, buf);
}

/*
 * Write an int on a fds
 * (uses ecrire)
 */
int ecrireLong (int fd, long i)
{
    char buf[64];
    sprintf(buf, "%ld", i);
    return ecrire(fd, buf);
}

/*
 * Write a char on a fds
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrireChar (int fd, char c)
{
    int pos = 0;
    while (pos < 1)
    {
        int i = write(fd, &c, 1);
        if (i == -1)
        {
            if (errno != EINTR)
                pos = 2;
        }
        else
            pos += i;

    }
    return pos != 1;
}

