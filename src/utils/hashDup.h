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

/* class hashTable
 * This class is in charge of making sure we don't crawl twice the same url
 */

#ifndef HASHDUP_H
#define HASHDUP_H

class hashDup
{
    private:
        ssize_t size;
        char *table;
        const char *file;

    public:
        /* constructor */
        hashDup (ssize_t size, const char *init, bool scratch);

        /* destructor */
        ~hashDup ();

        /*
         * set a page in the hashtable
         * return false if it was already there
         * return true if it was not (ie it is new)
         */
        bool testSet (char *doc);

        /* save in a file */
        void save ();
};

#endif // HASHDUP_H
