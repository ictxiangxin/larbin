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

#ifndef USEROUTPUT_H
#define USEROUTPUT_H

#include "utils/url.h"
#include "fetch/file.h"

/** A page has been loaded successfully
 * @param page the page that has been fetched
 */
void loaded (html *page);
  // This function should manage anything
  // page->getHeaders() gives a char* containing the http headers
  // page->getPage() gives a char* containing the page itself
  // those char* are statically allocated, so you should copy
  // them if you want to keep them
  // in order to accept \000 in the page, you can use page->getLength()

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void failure (url *u, FetchError reason);

/** initialisation function
 * This function is called at the end of global initialisation
 */
void initUserOutput ();

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
void outputStats(int fds);

#endif // USEROUTPUT_H
