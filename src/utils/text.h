// Larbin
// Sebastien Ailleret
// 18-11-99 -> 21-05-01

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
bool endWithIgnoreCase (const char *amin, const char *b, int lb);

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
