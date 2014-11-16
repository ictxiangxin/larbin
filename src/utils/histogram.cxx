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

#define SIZE 256
#define HEIGHT 15

#define HTTP(X) ecrire(fds, X)

/* definition of class histogram */
class Histogram
{
protected:
    int tab1[SIZE];
    int tab2[SIZE];
    int beg, end;
    int tick;
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
    for (int i = 0; i < SIZE; ++i)
    {
        tab1[i] = 0;
        tab2[i] = 0;
    }
    beg = 0;
    end = 0;
    this->period = period;
    count = 0;
    beg_time = time (NULL);
    tick = 0;
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
    if (++count >= period)
    {
        tick++;
        count = 0;
        incrementEnd();
    }
}

/* Insert a new interval : */
void Histogram::incrementEnd ()
{
    end++;
    if (end >= SIZE)
        end = 0;
    if (end <= beg)   /* have to delete the oldest interval */
    {
        beg++;
        if (beg >= SIZE)
            beg = 0;
        beg_time += period;
    }
    tab1[end] = 0;
    tab2[end] = 0;
}

/* Html output stat for last intervals */
void Histogram::write (int fds)
{
    HTTP("<script type=\"text/javascript\">\n");
    HTTP("g = new Dygraph(\n");
    HTTP("document.getElementById(\"graphdiv");
    ecrireInt(fds, period);
    HTTP("\"),\n");
    HTTP("\"Time, Success, Total\\n\" +\n");
    for (int i = beg, c = 0; (c < SIZE && c <= tick) || c < 10; ++i, ++c)
    {
        if (i == SIZE)
            i = 0;
        HTTP("\"");
        ecrireInt (fds, c);
        HTTP(", ");
        ecrireInt (fds, tab2[i]);
        HTTP(", ");
        ecrireInt (fds, tab1[i]);
        HTTP("\\n\" +\n");
    }
    HTTP("\"\",\n");
    HTTP("{title: 'Pages per ");
    ecrireInt(fds, period);
    HTTP("s', ");
    HTTP("ylabel: 'Pages'}\n");
    HTTP(");\n");
    HTTP("</script>\n");
}

