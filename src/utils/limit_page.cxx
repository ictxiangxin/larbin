
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
#include <time.h>
#include <pthread.h>

#include "types.h"
#include "global.h"
#include "utils/level.h"

void *pLimitPage (void *none)
{
    while(TRUE)
    {
        sleep(1);
        if(!global::searchOn)
            break;
        if(answers[success] >= global::limitPage)
        {
            std::cout << "["GREEN_MSG("Search")"] Limit pages." << std::endl;
            closeLevelUp(LEVEL_SEARCH);
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}
