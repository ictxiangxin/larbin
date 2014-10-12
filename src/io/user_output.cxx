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

/* This is the file you should change if you want to
 * use the data fetched by larbin.
 *
 * See useroutput.h for the interface
 *
 * See the files XXXuserouput.cc for examples */
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
#include "io/user_output_recall.h"

static char errorMsg[][32] =
{
    "Success",
    "No DNS",
    "No Connect",
    "Robots",
    "Timeout",
    "Bad Type",
    "Too Big",
    "Err 30X",
    "Err 40X",
    "Unknow",
    "Site Dup",
    "<fast>Robots",
    "<fast>No Connect",
    "<fast>No DNS",
    "Too Deep",
    "URL Dup",
    "Out Site"
};

void loaded (html *page)
{
    switch (global::outputMode)
    {
        case OM_DEFAULT :
            default_loaded(page);
            break;
        case OM_SAVE :
            save_loaded(page);
            break;
        case OM_MIRROR :
            mirror_loaded(page);
            break;
        case OM_STATS :
            stats_loaded(page);
            break;
        default :
            ;
    }
    if(global::fetchInfo)
    {
        std::cout << "["GREEN_MSG("Success")"] ";
        page->getUrl()->print();
    }
}

void failure (url *u, FetchError reason)
{
    switch (global::outputMode)
    {
        case OM_DEFAULT :
            default_failure(u, reason);
            break;
        case OM_SAVE :
            save_failure(u, reason);
            break;
        case OM_MIRROR :
            mirror_failure(u, reason);
            break;
        case OM_STATS :
            stats_failure(u, reason);
            break;
        default :
            ;
    }
    if(global::fetchInfo)
    {
        std::cout << "[" << RED_MSG(errorMsg[reason]) << "] ";
        u->print();
    }
}

void initUserOutput ()
{
    switch (global::outputMode)
    {
        case OM_DEFAULT :
            default_initUserOutput();
            break;
        case OM_SAVE :
            save_initUserOutput();
            break;
        case OM_MIRROR :
            mirror_initUserOutput();
            break;
        case OM_STATS :
            stats_initUserOutput();
            break;
        default :
            ;
    }
}

void outputStats (int fds)
{
    switch (global::outputMode)
    {
        case OM_DEFAULT :
            default_outputStats(fds);
            break;
        case OM_SAVE :
            save_outputStats(fds);
            break;
        case OM_MIRROR :
            mirror_outputStats(fds);
            break;
        case OM_STATS :
            stats_outputStats(fds);
            break;
        default :
            ;
    }
}
