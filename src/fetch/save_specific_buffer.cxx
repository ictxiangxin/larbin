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

#include <iostream>
#include <stdio.h>

#include "types.h"
#include "global.h"
#include "fetch/file.h"

static int  nbdir = -1;
static int  nbfile = filesPerDir;
static char *fileName;
static uint endFileName;
static int  indexFds = -1;
static char buf[maxUrlSize + 30];

/*
 * give the name of the file given dir and file number
 * this char * is static
 */
void getSpecName(int nbdir, int nbfile, int extindex)
{
    sprintf(fileName + endFileName, "d%5i/f%5i%s", nbdir, nbfile, global::privilegedExts[extindex]);
    for (uint i = endFileName + 1; fileName[i] == ' '; i++)
        fileName[i] = '0';
    for (uint i = endFileName + 8; fileName[i] == ' '; i++)
        fileName[i] = '0';
}

void getIndexName(int nbdir)
{
    sprintf(fileName + endFileName, "d%5i/index", nbdir);
    for (uint i = endFileName + 1; fileName[i] == ' '; i++)
        fileName[i] = '0';
}

/** in case of specific save */
void initSpecific ()
{
    mkdir(specDir, S_IRWXU);
    endFileName = strlen(specDir);
    fileName = new char[endFileName + 20];
    strcpy(fileName, specDir);
    if (fileName[endFileName - 1] != '/')
        fileName[endFileName++] = '/';
}

/** open file descriptor */
void html::newSpec ()
{
    nbfile++;
    if (nbfile >= filesPerDir) // new dir
    {
        nbdir++;
        nbfile = 0;
        // create the directory
        getIndexName(nbdir);
        fileName[endFileName + 6] = 0;
        if (mkdir(fileName, S_IRWXU) != 0)
            perror("Trouble while creating dir");
        fileName[endFileName + 6] = '/';
        // open new index
        close(indexFds);
        indexFds = creat(fileName, S_IRWXU);
        if (indexFds < 0)
        {
            std::cerr << "["RED_MSG("Error")"] Cannot open file " << fileName << " : " << strerror(errno) << std::endl;
            exit(-1);
        }
    }
    mydir = nbdir;
    myfile = nbfile;
    // write index
    int s = sprintf(buf, "%4d  ", nbfile);
    s += here->writeUrl(buf + s);
    buf[s++] = '\n';
    ecrireBuff(indexFds, buf, s);
    // open new file
    getSpecName(nbdir, nbfile, extIndex);
    fdsSpec = creat(fileName, S_IRWXU);
    if (fdsSpec < 0)
    {
        std::cerr << "["RED_MSG("Error")"] Cannot open file " << fileName << " : " << strerror(errno) << std::endl;
        exit(-1);
    }
    nbSpec = 0;
}

/** feed file descriptor */
bool html::pipeSpec ()
{
    int nb = buffer + pos - posParse;
    nbSpec += nb;
    if (nbSpec >= maxSpecSize)
    {
        errno = tooBig;
        return true;
    }
    ecrireBuff(fdsSpec, posParse, nb);
    pos = posParse - buffer;
    return false;
}

/** get the content of the page */
char *html::getContent ()
{
    static char content[maxSpecSize];
    if (mydir >= 0)
    {
        getSpecName(mydir, myfile, extIndex);
        int fds = open (fileName, O_RDONLY);
        if (fds < 0)
            perror(fileName);
        int cont = 1;
        int pos = 0;
        while (cont)
        {
            cont = read(fds, content + pos, maxSpecSize - 1 - pos);
            pos += cont;
        }
        content[pos] = 0;
        close(fds);
        return content;
    }
    else
        return contentStart;
}

