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
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "options.h"

#include "global.h"
#include "types.h"
#include "utils/url.h"
#include "utils/connection.h"
#include "fetch/hash_table.h"

/* constructor */
hashTable::hashTable (bool create)
{
    ssize_t total = hashSize >> 3;
    table = new char[total];
    if (create)
        for (ssize_t i = 0; i < hashSize >> 3; i++)
            table[i] = 0;
    else
    {
        int fds = open(hashFile, O_RDONLY);
        if (fds < 0)
        {
            std::cerr << "["YELLOW_MSG("Warning")"] Cannot find \""<< hashFile <<"\", restart from scratch" << std::endl;
            for (ssize_t i = 0; i < hashSize >> 3; i++)
                table[i] = 0;
        }
        else
        {
            ssize_t sr = 0;
            while (sr < total)
            {
                ssize_t tmp = read(fds, table + sr, total - sr);
                if (tmp <= 0)
                {
                    std::cerr << "["RED_MSG("Error")"] Cannot read \"" << hashFile << "\" : " << strerror(errno) << std::endl;
                    exit(-1);
                }
                else
                    sr += tmp;
            }
            close(fds);
        }
    }
}

/* destructor */
hashTable::~hashTable ()
{
    delete [] table;
}

/* save the hashTable in a file */
void hashTable::save()
{
    rename(hashFile, hashFileOld);
    int fds = creat(hashFile, 00600);
    if (fds >= 0)
    {
        ecrireBuff(fds, table, hashSize >> 3);
        close(fds);
    }
    unlink(hashFileOld);
}

/*
 * test if this url is allready in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool hashTable::test (url *U)
{
    uint code = U->hashCode();
    uint pos = code >> 3;
    uint bits = 1 << (code % 8);
    return table[pos] & bits;
}

/* set a url as present in the hashtable */
void hashTable::set (url *U)
{
    uint code = U->hashCode();
    uint pos = code >> 3;
    uint bits = 1 << (code % 8);
    table[pos] |= bits;
}

/*
 * add a new url in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool hashTable::testSet (url *U)
{
    uint code = U->hashCode();
    uint pos = code >> 3;
    uint bits = 1 << (code % 8);
    int res = table[pos] & bits;
    table[pos] |= bits;
    return !res;
}
