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

/*
 * class hashTable
 * This class is in charge of making sure we don't crawl twice the same url
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "types.h"
#include "utils/url.h"

class hashTable
{
    private:
        ssize_t size;
        char *table;

    public:
        /* constructor */
        hashTable (bool create);

        /* destructor */
        ~hashTable ();

        /* save the hashTable in a file */
        void save();

        /* test if this url is allready in the hashtable
         * return true if it has been added
         * return false if it has allready been seen
         */
        bool test (url *U);

        /* set a url as present in the hashtable */
        void set (url *U);

        /* add a new url in the hashtable
         * return true if it has been added
         * return false if it has allready been seen
         */
        bool testSet (url *U);
};

#endif // HASHTABLE_H
