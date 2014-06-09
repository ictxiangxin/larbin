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

/* this module defines how specific pages are handled
 * the following functions must be defined :
 * - initSpecific : this function is called when larbin is launched. This
 *     must be a function because it is defined in file.h.
 * - constrSpec : this function is called in the constructor of class html
 * - newSpec : a new specific page has been discovered
 * - pipeSpec : some data has arrived on the stream
 * - endInput : the end of the input : be careful, this function
 *     might not be called in case of timeout or other reasons
 * - getPage : someone needs the content of the page
 * - getSize : gives the size of the document
 * - destructSpec : this function is called in the destructor of class html
 *
 * All this function are only called if SPECIFICSEARCH is defined
 * constrSpec is called for every page
 * other functions are only called if state==SPECIFIC
 */

#ifdef SAVE_SPECIFIC
#include "fetch/savespecbuf.cc"

#elif defined(DYNAMIC_SPECIFIC)
#include "fetch/dynamicspecbuf.cc"

#else // DEFAULT_SPECIFIC
#include "fetch/defaultspecbuf.cc"

#endif
