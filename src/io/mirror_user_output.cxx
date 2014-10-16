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

static char *fileName;
static uint endFileName;

/** A page has been loaded successfully
 * @param page the page that has been fetched
 */
void mirror_loaded (html *page)
{
    // get file name and create needed directories
    url *u = page->getUrl();
    uint p = u->getPort();
    char *h = u->getHost();
    char *f = u->getFile();
    // update dir name
    uint d = u->hostHashCode() % nbDir;
    for (int i = 2; i < 7; i++)
    {
        fileName[endFileName - i] = d % 10 + '0';
        d /= 10;
    }
    // set file name
    uint len = endFileName;
    if (p == 80)
        len += sprintf(fileName + endFileName, "%s%s", h, f);
    else
        len += sprintf(fileName + endFileName, "%s:%u%s", h, p, f);
    // make sure the path of the file exists
    bool cont = true;
    struct stat st;
    while (cont)
    {
        len--;
        while (fileName[len] != '/')
            len--;
        fileName[len] = 0;
        cont = stat(fileName, &st); // this becomes true at least for saveDir
        fileName[len] = '/';
    }
    cont = true;
    while (cont)
    {
        len++;
        while (fileName[len] != '/' && fileName[len] != 0) len++;
        if (fileName[len] == '/')
        {
            fileName[len] = 0;
            if (mkdir(fileName, S_IRWXU) != 0) perror("trouble while creating dir");
            fileName[len] = '/';
        }
        else     // fileName[len] == 0
        {
            cont = false;
        }
    }
    if (fileName[len-1] == '/')
    {
        strcpy(fileName + len, indexFile);
    }
    // open fds and write file
    int fd = creat(fileName, S_IRWXU);
    if (fd >= 0)
    {
        // some url mysteries might prevent this from being possible
        // ex: if http://bbs.computoredge.com/ceo
        // and http://bbs.computoredge.com/ceo/ both exist
        ecrireBuff(fd, page->getPage(), page->getLength());
        close(fd);
    }
}

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void mirror_failure (url *u, FetchError reason)
{
}

/** initialisation function
 */
void mirror_initUserOutput ()
{
    mkdir(saveDir, S_IRWXU);
    endFileName = strlen(saveDir);
    fileName = new char[endFileName+maxUrlSize+50];
    strcpy(fileName, saveDir);
    if (fileName[endFileName - 1] != '/') fileName[endFileName++] = '/';
    strcpy(fileName + endFileName, "d00000/");
    endFileName += 7; // indique le premier char a ecrire
}

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
void mirror_outputStats(int fds)
{
    ecrire(fds, "Nothing to declare");
}
