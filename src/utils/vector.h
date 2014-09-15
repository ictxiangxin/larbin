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

#ifndef VECTOR_HH
#define VECTOR_HH

#include "types.h"

template <class T>
class Vector
{
    private:
        /** This array contain the object */
        T **tab;
        /** Number of object in the array */
        uint pos;
        /** Size of the array */
        uint size;

    public:
        /** Constructor */
        Vector (uint size = StdVectSize);
        /** Destructor */
        ~Vector ();
        /** Re-init this vector : empty it all */
        void recycle ();
        /** add an element to this vector */
        void addElement (T *elt);
        /** give the size of the vector */
        inline uint getLength () { return pos; }
        /** give the array containing the objects */
        inline T **getTab() { return tab; }
        /** get an element of this Vector */
        T *operator [] (uint i);
};

/** Constructor
 * @param size the initial capacity of the Vector
 */
template <class T>
Vector<T>::Vector (uint size)
{
    this->size = size;
    pos = 0;
    tab = new T*[size];
}

/** Destructor */
template <class T>
Vector<T>::~Vector ()
{
    for (uint i = 0; i < pos; i++)
	    delete tab[i];
    delete [] tab;
}

/** Re-init this vector : empty it all */
template <class T>
void Vector<T>::recycle ()
{
    for (uint i = 0; i < pos; i++)
	    delete tab[i];
    pos = 0;
}

/** add an element to this vector */
template <class T>
void Vector<T>::addElement (T *elt)
{
    assert (pos <= size);
    if (pos == size)
    {
	    size *= 2;
	    T **tmp = new T*[size];
	    for (uint i = 0; i < pos; i++)
	        tmp[i] = tab[i];
	    delete [] tab;
	    tab = tmp;
    }
    tab[pos] = elt;
    pos++;
}

/** get an element of this Vector */
template <class T>
T *Vector<T>::operator [] (uint i)
{
    if (i < pos)
	    return tab[i];
    else
	    return NULL;
}

#endif // VECTOR_HH
