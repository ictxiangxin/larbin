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
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>

#include "types.h"
#include "global.h"
#include "utils/thread.h"
#include "utils/persistent_fifo.h"

PersistentFifo::PersistentFifo (bool reload, char *baseName)
{
    fileNameLength = strlen(baseName) + 5;
    fileName = new char[fileNameLength + 2];
    strcpy(fileName, baseName);
    fileName[fileNameLength + 1] = 0;
    outbufPos = 0;
    bufPos = 0;
    bufEnd = 0;
    mypthread_mutex_init (&lock, NULL);
    if (reload)
    {
        DIR *dir = opendir(".");
        struct dirent *name;

        fin = -1;
        fout = -1;
        name = readdir(dir);
        while (name != NULL)
        {
            if (startWith(fileName, name->d_name))
            {
                int tmp = getNumber(name->d_name);
                if (fin == -1)
                {
                    fin = tmp;
                    fout = tmp;
                }
                else
                {
                    if (tmp > fin)
                        fin = tmp;
                    if (tmp < fout)
                        fout = tmp;
                }
            }
            name = readdir(dir);
        }
        if (fin == -1)
        {
            fin = 0;
            fout = 0;
        }
        if (fin == fout && fin != 0)
        {
            std::cerr << "["RED_MSG("Error")"] previous crawl was too little, cannot reload state"
                      << std::endl
                      << "please restart larbin with --scratch option"
                      << std::endl;
            exit(-1);
        }
        closedir(dir);
        in = (fin - fout) * urlByFile;
        out = 0;
        makeName(fin);
        wfds = creat (fileName, S_IRUSR | S_IWUSR);
        makeName(fout);
        rfds = open (fileName, O_RDONLY);
    }
    else
    {
        // Delete old fifos
        DIR *dir = opendir(".");
        struct dirent *name;
        name = readdir(dir);
        while (name != NULL)
        {
            if (startWith(fileName, name->d_name))
                unlink(name->d_name);
            name = readdir(dir);
        }
        closedir(dir);

        fin = 0;
        fout = 0;
        in = 0;
        out = 0;
        makeName(0);
        wfds = creat (fileName, S_IRUSR | S_IWUSR);
        rfds = open (fileName, O_RDONLY);
    }
}

PersistentFifo::~PersistentFifo ()
{
    mypthread_mutex_destroy (&lock);
    close(rfds);
    close(wfds);
}

url *PersistentFifo::tryGet ()
{
    url *tmp = NULL;
    mypthread_mutex_lock(&lock);
    if (in != out)
    {
        // The stack is not empty
        char *line = readLine();
        tmp = new url(line);
        out++;
        updateRead();
    }
    mypthread_mutex_unlock(&lock);
    return tmp;
}

url *PersistentFifo::get ()
{
    mypthread_mutex_lock(&lock);
    char *line = readLine();
    url *res = new url(line);
    out++;
    updateRead();
    mypthread_mutex_unlock(&lock);
    return res;
}

/** Put something in the fifo
 * The objet is then deleted
 */
void PersistentFifo::put (url *obj)
{
    mypthread_mutex_lock(&lock);
    char *s = obj->serialize(); // statically allocated string
    writeUrl(s);
    in++;
    updateWrite();
    mypthread_mutex_unlock(&lock);
    delete obj;
}

int PersistentFifo::getLength ()
{
    return in - out;
}

void PersistentFifo::makeName (uint nb)
{
    for (uint i = fileNameLength; i >= fileNameLength - 5; i--)
    {
        fileName[i] = (nb % 10) + '0';
        nb /= 10;
    }
}

int PersistentFifo::getNumber (char *file)
{
    uint len = strlen(file);
    int res = 0;
    for (uint i = len - 6; i <= len - 1; i++)
        res = (res * 10) + file[i] - '0';
    return res;
}

void PersistentFifo::updateRead ()
{
    if ((out % urlByFile) == 0)
    {
        close(rfds);
        makeName(fout);
        unlink(fileName);
        makeName(++fout);
        rfds = open(fileName, O_RDONLY);
        in -= out;
        out = 0;
        assert(bufPos == bufEnd);
    }
}

void PersistentFifo::updateWrite ()
{
    if ((in % urlByFile) == 0)
    {
        flushOut();
        close(wfds);
        makeName(++fin);
        wfds = creat(fileName, S_IRUSR | S_IWUSR);
        if(global::canReload)
        {
            global::seen->save();
            if (global::pageNoDuplicate)
                global::hDuplicate->save();
        }
    }
}

/* read a line from the file
 * uses a buffer
 */
char *PersistentFifo::readLine ()
{
    if (bufPos == bufEnd)
    {
        bufPos = 0;
        bufEnd = 0;
        buf[0] = 0;
    }
    char *posn = strchr(buf + bufPos, '\n');
    while (posn == NULL)
    {
        if (!(bufEnd - bufPos < maxUrlSize + 40 + maxCookieSize))
        {
            std::cout << fileName << std::endl;
            std::cout << buf + bufPos << std::endl;

        }
        if (bufPos * 2 > BUF_SIZE)
        {
            bufEnd -= bufPos;
            memmove(buf, buf + bufPos, bufEnd);
            bufPos = 0;
        }
        int postmp = bufEnd;
        bool noRead = true;
        while (noRead)
        {
            int rd = read(rfds, buf + bufEnd, BUF_SIZE - 1 - bufEnd);
            switch (rd)
            {
            case 0 :
                // We need to flush the output in order to read it
                flushOut();
                break;
            case -1 :
                // We have a trouble here
                if (errno != EINTR)
                {
                    std::cerr << "["RED_MSG("Error")"] Big Problem while reading (persistentFifo.h)\n";
                    perror("reason");
                    assert(false);
                }
                else
                    perror("Warning in PersistentFifo: ");
                break;
            default:
                noRead = false;
                bufEnd += rd;
                buf[bufEnd] = 0;
                break;
            }
        }
        posn = strchr(buf + postmp, '\n');
    }
    *posn = 0;
    char *res = buf + bufPos;
    bufPos = posn + 1 - buf;
    return res;
}

// write an url in the out file (buffered write)
void PersistentFifo::writeUrl (char *s)
{
    size_t len = strlen(s);
    assert(len < maxUrlSize + 40 + maxCookieSize);
    if (outbufPos + len < BUF_SIZE)
    {
        memcpy(outbuf + outbufPos, s, len);
        outbufPos += len;
    }
    else
    {
        // The buffer is full
        flushOut ();
        memcpy(outbuf + outbufPos, s, len);
        outbufPos = len;
    }
}

// Flush the out Buffer in the outFile
void PersistentFifo::flushOut ()
{
    ecrireBuff (wfds, outbuf, outbufPos);
    outbufPos = 0;
}
