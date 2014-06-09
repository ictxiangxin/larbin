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

/* This is the file you should change if you want to
 * use the data fetched by larbin.
 *
 * See useroutput.h for the interface
 *
 * See the files XXXuserouput.cc for examples */

#include "options.h"

#ifdef SIMPLE_SAVE
#include "interf/saveuseroutput.cc"

#elif defined(MIRROR_SAVE)
#include "interf/mirrorsaveuseroutput.cc"

#elif defined(STATS_OUTPUT)
#include "interf/statsuseroutput.cc"

#else // DEFAULT_OUTPUT
#include "interf/defaultuseroutput.cc"

#endif
