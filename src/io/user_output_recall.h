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

#ifndef _USEROUTPUTRECALL_H_
#define _USEROUTPUTRECALL_H_

void default_loaded (html *page);
void save_loaded (html *page);
void mirror_loaded (html *page);
void stats_loaded (html *page);
void default_failure (url *u, FetchError reason);
void save_failure (url *u, FetchError reason);
void mirror_failure (url *u, FetchError reason);
void stats_failure (url *u, FetchError reason);
void default_initUserOutput ();
void save_initUserOutput ();
void mirror_initUserOutput ();
void stats_initUserOutput ();
void default_outputStats(int fds);
void save_outputStats(int fds);
void mirror_outputStats(int fds);
void stats_outputStats(int fds);

#endif
