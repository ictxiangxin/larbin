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
#include "interf/useroutput.h"
#include "utils/mypthread.h"


/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void fetchFail (url *u, FetchError err, bool interesting=false)
{
#ifdef SPECIFICSEARCH
    if (interesting || (privilegedExts[0] != NULL && matchPrivExt(u->getFile())))
#endif
        failure(u, err);
}

/** It's over with this file
 * report the situation ! (and make some stats)
 */
void endOfLoad (html *parser, FetchError err)
{
    answers(err);
    switch (err)
    {
    case success:
#ifdef SPECIFICSEARCH
        if (parser->isInteresting)
        {
            interestingPage();
            loaded(parser);
        }
#else // not a SPECIFICSEARCH
        loaded(parser);
#endif // SPECIFICSEARCH
        break;
    default:
        fetchFail(parser->getUrl(), err, parser->isInteresting);
        break;
    }
}

#ifdef THREAD_OUTPUT
/** In this thread, end user manage the result of the crawl
 */
static void *startOutput (void *none)
{
    initUserOutput();
    while (true)
    {
        Connexion *conn = global::userConns->get();
        endOfLoad((html *)conn->parser, conn->err);
        conn->recycle();
        global::freeConns->put(conn);
    }
    return NULL;
}

void initOutput ()
{
    startThread(startOutput, NULL);
}

#else // THREAD_OUTPUT not defined

void initOutput ()
{
    initUserOutput();
}

#endif // THREAD_OUTPUT
