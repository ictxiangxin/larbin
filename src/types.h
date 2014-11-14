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

#ifndef TYPES_H
#define TYPES_H

#define TRUE  1
#define FALSE 0

// Size of the HashSize (max number of urls that can be fetched)
#define hashSize 64000000
#define hashFile "hashtable.bin"
#define hashFileOld "hashtable.old"

// Size of the duplicate hashTable
#define dupSize hashSize
#define dupFile "dupfile.bin"

// Size of the arrays of Sites in main memory
#define namedSiteListSize 20000
#define IPSiteListSize 10000

// Max number of urls in ram
#define ramUrls   100000
#define maxIPUrls 80000  // this should allow less dns call

// Max number of urls per site in Url
#define maxUrlsBySite 64  // must fit in uint8_t

// time out when reading a page (in sec)
#define timeoutPage 30   // default time out
#define timeoutIncr 2000 // number of bytes for 1 more sec

// How long do we keep dns answers and robots.txt
#define dnsValidTime (2 * 24 * 3600)

// Maximum size of a page
#define maxPageSize    8 * 1024 * 1024
#define nearlyFullPage (maxPageSize - 512 * 1024)

// Maximum size of a robots.txt that is read
// the value used is min(maxPageSize, maxRobotsSize)
#define maxRobotsSize 64 * 1024

// How many forbidden items do we accept in a robots.txt
#define maxRobotsItem 256

// file name used for storing urls on disk
#define fifoFile     "fifo"
#define fifoFileWait "fifowait"

// number of urls per file on disk
// should be equal to ramUrls for good interaction with restart
#define urlByFile ramUrls

// Size of the buffer used to read sockets
#define BUF_SIZE    64 * 1024
#define STRING_SIZE 1024

// Max size for a url
#define maxUrlSize  1024
#define maxSiteSize 256   // max size for the name of a site

// max size for cookies
#define maxCookieSize 128

// Standard size of a fifo in a Site
#define StdVectSize maxRobotsItem

// maximum number of input connections
#define maxInput 5

// if we save files, how many files per directory and where
#define filesPerDir 2000
#define saveDir     "save/"
#define indexFile   "index.html"    // for MIRROR_SAVE
#define nbDir       1000            // for MIRROR_SAVE

// options for SPECIFICSEARCH (except with DEFAULT_SPECIFIC)
#define specDir     "specific/"
#define maxSpecSize 32 * 1024 * 1024

// Various reasons of error when getting a page
#define nbAnswers 16

#define RED_MSG(msg) "\e[0;31m" << msg << "\e[0m"
#define GREEN_MSG(msg) "\e[0;32m" << msg << "\e[0m"
#define YELLOW_MSG(msg) "\e[0;33m" << msg << "\e[0m"

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

enum FetchError
{
    success,
    noDNS,
    noConnection,
    forbiddenRobots,
    timeout,
    badType,
    tooBig,
    err30X,
    err40X,
    earlyStop,
    duplicate,
    fastRobots,
    fastNoConn,
    fastNoDns,
    tooDeep,
    urlDup,
    outSite
};

// output mode
#define OM_DEFAULT 0
#define OM_SAVE    1
#define OM_MIRROR  2
#define OM_STATS   3

// level
#define LEVEL_ALL -1
#define LEVEL_SEARCH 0
#define LEVEL_WEBSERVER 1

// standard types
typedef unsigned int        uint;
typedef unsigned short int  ushort;

#endif // TYPES_H
