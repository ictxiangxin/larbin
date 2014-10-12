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

#include "config.h"

#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "utils/hash_duplicate.h"
#include "utils/connection.h"

// constructor
hashDup::hashDup (ssize_t size, const char *init, bool scratch)
{
    this->size = size;
    file = init;
    table = new char[size / 8];
    if (init == NULL || scratch)
    {
        for (ssize_t i = 0; i < size / 8; i++)
            table[i] = 0;
    }
    else
    {
        int fds = open(init, O_RDONLY);
        if (fds < 0)
        {
            std::cerr << "["YELLOW_MSG("Warning")"] Cannot find \"" << init << "\", restart from scratch." << std::endl;
            for (ssize_t i = 0; i < size / 8; i++)
                table[i] = 0;
        }
        else
        {
            ssize_t sr = 0;
            while (sr < size)
            {
                ssize_t tmp = read(fds, table + sr, size - sr);
                if (tmp <= 0)
                {
                    std::cerr << "["RED_MSG("Error")"] Cannot read \"" << init << "\"" << std::endl;
                    exit(-1);
                }
                else
                    sr += 8 * tmp;
            }
            close(fds);
        }
    }
}

// destructor
hashDup::~hashDup ()
{
    delete [] table;
}

/*
 * set a page in the hashtable
 * return false if it was already there
 * return true if it was not (ie it is new)
 */
bool hashDup::testSet (char *doc)
{
    uint code = 0;
    char c;
    for (uint i = 0; (c = doc[i])!= 0; i++)
        if (c > 'A' && c < 'z')
            code = (code * 23 + c) % size;
    uint pos = code / 8;
    uint bits = 1 << (code % 8);
    int res = table[pos] & bits;
    table[pos] |= bits;
    return !res;
}

// save in a file
void hashDup::save ()
{
    int fds = creat(file, 00600);
    if (fds >= 0)
    {
        ecrireBuff(fds, table, size / 8);
        close(fds);
    }
}
