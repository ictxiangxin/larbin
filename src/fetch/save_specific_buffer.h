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

void getSpecName(int nbdir, int nbfile, int extindex);
void getIndexName(int nbdir);
void initSpecific ();

#define getSize() nbSpec

#define constrSpec() \
            do { \
                fdsSpec = -1; \
                mydir = -1; \
            } while(0)

#define endOfInput() 0

#define destructSpec() \
            do { \
                if (fdsSpec != -1) \
                    close(fdsSpec); \
            } while(0)
