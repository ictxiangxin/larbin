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
#include <stdlib.h>

#include "utils/thread.h"

/* Launch a new thread
 * return 0 in case of success
 */
pthread_t startThread (StartFun run, void *arg)
{
    pthread_t t;
    pthread_attr_t attr;
    if (
           pthread_attr_init(&attr) != 0
        || pthread_create(&t, &attr, run, arg) != 0
        || pthread_attr_destroy(&attr) != 0
        || pthread_detach(t) != 0
       )
    {
        std::cerr << "Unable to launch a thread" << std::endl;
        exit(1);
    }
    return t;
}
