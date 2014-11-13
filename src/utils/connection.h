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

#ifndef CONNEXION_H
#define CONNEXION_H

/* make write until everything is written
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrire (int fd, const char *buf);

/* make write until everything is written
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrireBuff (int fd, const char *buf, int count);

/** Write an int on a fds
 * (uses ecrire)
 */
int ecrireInt (int fd, int i);
int ecrireInt2 (int fd, int i);
int ecrireInti (int fd, int i, const char *f);
int ecrireIntl (int fd, long i, const char *f);

/** Write an int on a fds
 * (uses ecrire)
 */
int ecrireLong (int fd, long i);

/* Write a char on a fds
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrireChar (int fd, char c);


#endif // CONNEXION_H
