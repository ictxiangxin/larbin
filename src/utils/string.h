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

#ifndef STRING_HH
#define STRING_HH

#include <assert.h>

#include "types.h"
#include "utils/debug.h"

class LarbinString
{
    private:
        char *chaine;
        uint pos;
        uint size;
    public:
        // Constructor
        LarbinString (uint size=STRING_SIZE);
        // Destructor
        ~LarbinString ();
        // Recycle this string
        void recycle (uint size=STRING_SIZE);
        // get the char * : it is deleted when you delete this String Object
        char *getString ();
        // give a char * : it creates a new one : YOU MUST DELETE IT YOURSELF
        char *giveString ();
        // append a char
        void addChar (char c);
        // append a char *
        void addString (const char *s);
        // append a buffer
        void addBuffer (const char *s, uint len);
        // length of this string
        inline uint getLength ()
        {
            return pos;
        }
        // get a char of this string
        inline char operator [] (uint i)
        {
            assert(i <= pos);
            return chaine[i];
        }
        // change a char
        void setChar (uint i, char c);
};

#endif // STRING_HH
