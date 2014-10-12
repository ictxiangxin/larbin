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

/* histogram of number of pages retrieved for graphical stats */

#include "options.h"

#include <string.h>

#include "global.h"
#include "utils/histogram.h"
#include "utils/connection.h"

#define SIZE 80
#define HEIGHT 15

static char curve[HEIGHT + 2][SIZE + 17];

/* definition of class histogram */
class Histogram
{
protected:
    int tab1[SIZE];
    int tab2[SIZE];
    int maxv;
    int beg, end;
    time_t period, count, beg_time;

    /* Insert a new interval : */
    void incrementEnd ();

public:
    /* Specific constructor : the number of pages retrieved during
     *   an interval is traced
     * @param period : time length of an interval (in seconds)
     * @param SIZE : number of intervals remembered
     */
    Histogram (time_t period);

    /* Destructor */
    ~Histogram ();

    /* A page is retrieved, add to stats */
    void pageHit (int x, int y);

    /* Text output stat for last intervals */
    void write (int fds);
};

/* our histograms */
static Histogram *histoHeure = new Histogram (3600);
static Histogram *histoMinute = new Histogram (60);
static Histogram *histoSeconde = new Histogram (1);

void histoHit (uint x, uint y)
{
    static uint lastx = 0, lasty = 0;
    int tmpx = x - lastx;
    int tmpy = y - lasty;
    lastx = x;
    lasty = y;
    histoHeure->pageHit (tmpx, tmpy);
    histoMinute->pageHit (tmpx, tmpy);
    histoSeconde->pageHit (tmpx, tmpx);
}

void histoWrite (int fds)
{
    histoHeure->write (fds);
    histoMinute->write (fds);
    histoSeconde->write (fds);
}

/*************************************/
/* Implementation of class histogram */
/*************************************/

/* Specific constructor : the number of pages retrieved during
 *   an interval is traced
 * @param period : time length of an interval (in seconds)
 * @param SIZE : number of intervals remembered
 */
Histogram::Histogram (time_t period)
{
    for (int i=0; i<SIZE; ++i)
    {
        tab1[i] = 0;
        tab2[i] = 0;
    }
    beg = 0;
    end = 0;
    maxv = HEIGHT;
    this->period = period;
    count = 0;
    beg_time = time (NULL);
    for (int i=0; i<HEIGHT; i++)
    {
        sprintf(curve[i], "                ");
        curve[i][SIZE+16] = '\n';
    }
    curve[0][13] = '-';
    curve[0][14] = '>';
    sprintf(curve[HEIGHT], "           0 -> ");
    for (int i=16; i<SIZE+16; i++) curve[HEIGHT][i] = '-';
    curve[HEIGHT][SIZE+16] = 0;
}

/* Destructor */
Histogram::~Histogram ()
{
}

/* A page is retrieved, add to stats */
void Histogram::pageHit (int x, int y)
{
    tab1[end] += x;
    tab2[end] += y;
    if (tab1[end] > maxv) maxv = tab1[end];
    if (++count >= period)
    {
        count = 0;
        incrementEnd();
    }
}

/* Insert a new interval : */
void Histogram::incrementEnd ()
{
    end += 1;
    if (end >= SIZE) end = 0;
    if (end <= beg)   /* have to delete the oldest interval */
    {
        beg += 1;
        if (beg >= SIZE) beg = 0;
        beg_time += period;
    }
    tab1[end] = 0;
    tab2[end] = 0;
}

/* Html output stat for last intervals */
void Histogram::write (int fds)
{
    /* Compute the curve */
    int maxvbis = maxv;
    maxv = HEIGHT; /* let's recompute it for next time */
    for (int i=beg, c=0; c<SIZE; ++i, ++c)
    {
        if (i == SIZE) i = 0;
        if (tab1[i] > maxv) maxv = tab1[i];
        int h1 = (tab1[i] * HEIGHT) / maxvbis;
        int h2 = (tab2[i] * HEIGHT) / maxvbis;
        for (int j=0; j<HEIGHT; ++j)
        {
            if (j >= h1)
                curve[HEIGHT-1-j][c+16] = ' ';
            else if (j >= h2)
                curve[HEIGHT-1-j][c+16] = 'x';
            else
                curve[HEIGHT-1-j][c+16] = '*';
        }
    }

    /* Write the curve */
    ecrire (fds, (char*)"\n\nOne column is the number of pages retrieved during ");
    ecrireInt (fds, period);
    ecrire (fds, (char*)" seconds : \n");
    sprintf(curve[0], "%12d", maxvbis);
    curve[0][12] = ' ';
    int now_col = (global::now - beg_time) / period + 16;
    curve[HEIGHT][now_col] = '|';
    ecrire(fds, curve[0]);
    curve[HEIGHT][now_col] = '-';
    ecrireChar(fds, '\n');
    /* Show time bounds : */
    char *deb = asctime (localtime (&beg_time));
    deb[strlen(deb)-1] = 0;
    ecrire (fds, deb);
    snprintf(curve[HEIGHT+1], SIZE + 3 - strlen(deb), "%10000s", "");
    ecrire(fds, curve[HEIGHT+1]);
    time_t fin_time = beg_time + period * SIZE;
    char *fin = asctime (localtime (&fin_time));
    ecrire (fds, fin);
}

