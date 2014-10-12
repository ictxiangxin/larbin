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
#include <string.h>
#include <unistd.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "fetch/file.h"
#include "utils/text.h"
#include "utils/debug.h"
#include "io/output.h"


static int nbdir = -1;
static int nbfile = filesPerDir;
static char buf[maxUrlSize+30];
static char *fileName;
static int indexFds = -1;
static uint endFileName;

/** A page has been loaded successfully, save it to disk
 * @param page the page that has been fetched
 */
void save_loaded (html *page)
{
    nbfile++;
    if (nbfile < filesPerDir)
    {
        int tmp = nbfile;
        int pos = endFileName;
        for (; tmp != 0; tmp /= 10)
            fileName[pos--] = '0' + (tmp % 10);
    }
    else // new dir
    {
        // create the directory
        nbdir++;
        nbfile = 0;
        int pos = endFileName - 7;
        int tmp = nbdir;
        for (; tmp != 0; tmp /= 10)
            fileName[pos--] = '0' + (tmp % 10);
        fileName[endFileName - 6] = 0;
        if (mkdir(fileName, S_IRWXU) != 0)
            perror("Trouble while creating dir");
        fileName[endFileName - 6] = '/';
        // open new index
        close(indexFds);
        strcpy(fileName + endFileName - 5, "index");
        indexFds = creat(fileName, S_IRWXU);
        if (indexFds < 0)
        {
            std::cerr << "["RED_MSG("Error")"] Cannot open file " << fileName << std::endl;
            exit(-1);
        }
        // new filename
        fileName[endFileName - 5] = 'f';
        for (uint i = endFileName; i > endFileName - 5; i--)
            fileName[i] = '0';
    }
    int fd = creat(fileName, S_IRWXU);
    if (fd < 0)
    {
        std::cerr << "["RED_MSG("Error")"] Cannot open file " << fileName << std::endl;
        exit(-1);
    }
    int s = 0;
    s = sprintf(buf, "%4u ", nbfile);
#ifdef URL_TAGS
    s += sprintf(buf + s, "(%u) ", page->getUrl()->tag);
#endif // URL_TAGS
    s += page->getUrl()->writeUrl(buf+s);
    buf[s++] = '\n';
    ecrireBuff(indexFds, buf, s);
    ecrireBuff(fd, page->getPage(), page->getLength());
    close(fd);
}

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void save_failure (url *u, FetchError reason)
{
    // do nothing
}

/** initialisation function
 */
void save_initUserOutput ()
{
    mkdir(saveDir, S_IRWXU);
    endFileName = strlen(saveDir);
    fileName = new char[endFileName+maxUrlSize + 50];
    strcpy(fileName, saveDir);
    if (fileName[endFileName - 1] != '/')
        fileName[endFileName++] = '/';
    strcpy(fileName + endFileName, "d00000/f00000");
    endFileName += 12; // indique le dernier char ecrit
}

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
void save_outputStats(int fds)
{
    ecrire(fds, "Nothing to declare");
}
