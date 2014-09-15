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

#include <string.h>
#include <iostream>

#include "options.h"

#include "utils/text.h"
#include "utils/string.h"


// Constructor
LarbinString::LarbinString (uint size)
{
    chaine = new char[size];
    this->size = size;
    pos = 0;
    chaine[0] = '\0';
}

// Destructor
LarbinString::~LarbinString ()
{
    delete [] chaine;
}

// Recycle this string
void LarbinString::recycle (uint size)
{
    if (this->size > size)
    {
        delete [] chaine;
        chaine = new char[size];
        this->size = size;
    }
    pos = 0;
    chaine[0] = '\0';
}

// get the char *
char *LarbinString::getString ()
{
    return chaine;
}

/** give a new string (allocate a new one
 * the caller will have to delete it
 */
char *LarbinString::giveString ()
{
    return newString(chaine);
}

// append a char
void LarbinString::addChar (char c)
{
    chaine[pos] = c;
    pos++;
    if (pos >= size)
    {
        size *= 2;
        char *tmp = new char[size];
        memcpy(tmp, chaine, pos);
        delete [] chaine;
        chaine = tmp;
    }
    chaine[pos] = '\0';
}

// append a char *
void LarbinString::addString (const char *s)
{
    uint len = strlen(s);
    addBuffer(s, len);
}

// append a buffer
void LarbinString::addBuffer (const char *s, uint len)
{
    if (size <= pos + len)
    {
        size *= 2;
        if (size <= pos + len)
            size = pos + len + 1;
        char *tmp = new char[size];
        memcpy(tmp, chaine, pos);
        delete [] chaine;
        chaine = tmp;
    }
    memcpy(chaine + pos, s, len);
    pos += len;
    chaine[pos] = '\0';
}

// change a char
void LarbinString::setChar (uint i, char c)
{
    chaine[i] = c;
}
