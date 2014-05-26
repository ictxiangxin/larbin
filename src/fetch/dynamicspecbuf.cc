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

// use dynamic buffers when downloading specific pages

void initSpecific () { }

#define newSpec() ((void) 0)
#define endOfInput() 0

bool html::pipeSpec ()
{
    if (dynbuf == NULL)
    {
        if (pos > nearlyFullPage)
        {
            // need a dyn buf (big file)
            szDyn = 2*maxPageSize;
            dynbuf = new char[szDyn];
            nbSpec = buffer + pos - posParse;
            memcpy(dynbuf, posParse, nbSpec);
            dynbuf[nbSpec] = 0;
            pos = posParse - buffer;
        }
        return false;
    }
    else
    {
        int nb = buffer + pos - posParse;
        int newnb = nbSpec + nb;
        if (newnb >= maxSpecSize)
        {
            errno = tooBig;
            return true;
        }
        if (newnb >= szDyn)
        {
            // resize buffer
            szDyn *= 2;
            char *tmp = new char[szDyn];
            memcpy(tmp, dynbuf, nbSpec);
            delete[] dynbuf;
            dynbuf = tmp;
        }
        memcpy(dynbuf+nbSpec, posParse, nb);
        nbSpec = newnb;
        dynbuf[nbSpec] = 0;
        pos = posParse - buffer;
        return false;
    }
}

char * html::getContent()
{
    if (dynbuf != NULL)
        return dynbuf;
    else
        return contentStart;
}

int html::getSize()
{
    if (dynbuf != NULL)
    {
        return nbSpec;
    }
    else
    {
        return (buffer + pos - contentStart);
    }
}

#define constrSpec() dynbuf = NULL

#define destructSpec() if (dynbuf != NULL) { delete[] dynbuf; }
