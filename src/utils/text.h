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

#ifndef TEXT_H
#define TEXT_H

#include "utils/string.h"

/* lowercase a char */
char lowerCase (char a);

/* tests if b starts with a */
bool startWith (const char *a, const char *b);

/* test if b is forbidden by pattern a */
bool robotsMatch (const char *a, const char *b);

/* tests if b starts with a ignoring case */
bool startWithIgnoreCase (const char *a, const char *b);

/* test if b end with a */
bool endWith (const char *a, const char *b);

/* test if b end with a ignoring case
 * a can use min char, '.' (a[i] = a[i] | 32)
 */
bool endWithIgnoreCase (const char *amin, const char *b, uint lb);

/* tests if b contains a */
bool caseContain (const char *a, const char *b);

/* create a copy of a string */
char *newString (const char *arg);

/* Read a whole file
 */
char *readfile (int fds);

/* find the next token in the robots.txt, or in config file
 * must delete comments
 * no allocation (cf strtok); content is changed
 * param c is a bad hack for using the same function for robots and configfile
 */
char *nextToken(char **posParse, char c=' ');

/* does this char * match privilegedExt */
bool matchPrivExt(const char *file);

/* does this char * match contentType */
int matchContentType(const char *ct);

#endif // TEXT_H
