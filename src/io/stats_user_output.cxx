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

#include <iostream>
#include <string.h>
#include <unistd.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "fetch/file.h"
#include "utils/text.h"
#include "utils/debug.h"
#include "io/output.h"

#define nb 25
#define taille 4096
#define larg 40

static double maxs = 0;
static double tabs[nb];

static double maxb = 0;
static double tabb[nb];

static uint64_t totalpages = 1;
static double totalbytes = 0;

/** A page has been loaded successfully
 * @param page the page that has been fetched
 */
void stats_loaded (html *page)
{
    uint32_t l = page->getLength();
    int t = l / taille;
    if (t >= nb)
    {
        t = nb-1;
    }
    tabs[t]++;
    if (tabs[t] > maxs) maxs = tabs[t];
    tabb[t] += (double) l;
    if (tabb[t] > maxb) maxb = tabb[t];
    totalpages++;
    totalbytes += (double) l;
}

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void stats_failure (url *u, FetchError reason)
{
}

/** initialisation function
 */
void stats_initUserOutput ()
{
    for (int i=0; i<nb; i++)
    {
        tabs[i] = 0;
        tabb[i] = 0;
    }
}

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
static void dessine(int fds, double *tab, double *maxi)
{
    for (int i=0; i<nb; i++)
    {
        ecrire(fds, (char*)"|");
        int n = (int) ((tab[i] * larg) / (*maxi+1));
        for (int j=0; j<n; j++) ecrire(fds, (char*)"*");
        ecrire(fds, (char*)"\n");
    }
}

void stats_outputStats(int fds)
{
    ecrire(fds, (char*)"Stats for ");
    ecrireInt(fds, totalpages);
    ecrire(fds, (char*)" pages.\nMean size of a page : ");
    ecrireInt(fds, ((int) totalbytes) / totalpages);
    ecrire(fds, (char*)"\n\nProportion of pages per size (one row is ");
    ecrireInt(fds, taille);
    ecrire(fds, (char*)" bytes, max size is ");
    ecrireInt(fds, taille*nb);
    ecrire(fds, (char*)" bytes) :\n\n");
    dessine(fds, tabs, &maxs);
    ecrire(fds, (char*)"\n\nbytes transfered by size :\n\n");
    dessine(fds, tabb, &maxb);
}
