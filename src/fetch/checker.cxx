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

/* This modules is a filter
 * given some normalized URLs, it makes sure their extensions are OK
 * and send them if it didn't see them before
 */

#include <iostream>
#include <string.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/url.h"
#include "utils/text.h"
#include "utils/vector.h"
#include "fetch/hash_table.h"
#include "fetch/file.h"

#include "utils/debug.h"

/*
 * check if an url is allready known
 * if not send it
 * @param u the url to check
 */
void check (url *u)
{
    if (global::seen->testSet(u))
    {
        hashUrls();  // stat
        // where should this link go ?
        if (global::specificSearch && global::privilegedExts[0] != NULL && matchPrivExt(u->getFile()))
        {
            interestingExtension();
            global::URLsPriority->put(u);
        }
        else
            global::URLsDisk->put(u);
    }
    else
    {
        // This url has already been seen
        answers(urlDup);
        delete u;
    }
}

/*
 * Check the extension of an url
 * @return true if it might be interesting, false otherwise
 */
bool filter1 (char *host, char *file)
{
    if (global::domains != NULL)
    {
        bool ok = false;
        for (int i = 0; (*global::domains)[i] != NULL; i++)
            ok = ok || endWith((*global::domains)[i], host);
        if (!ok)
            return false;
    }
    int l = strlen(file);
    if (   endWithIgnoreCase((char*)"html", file, l)
            || endWithIgnoreCase((char*)"htm", file, l)
            || file[l - 1] == '/'
       )
        return true;
    for (int i = 0; global::forbExt[i] != NULL; i++)
        if (endWithIgnoreCase(global::forbExt[i], file, l))
            return false;
    return true;
}
