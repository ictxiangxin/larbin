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

#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

#include "options.h"

#include "global.h"

#include "fetch/checker.h"
#include "fetch/sequencer.h"
#include "fetch/fetchOpen.h"
#include "fetch/fetchPipe.h"

#include "interf/input.h"
#include "interf/output.h"

#include "utils/text.h"
#include "utils/mypthread.h"
#include "utils/debug.h"
#include "utils/webserver.h"
#include "utils/histogram.h"
#include "utils/limitTime.h"
#include "utils/level.h"

static int cron ();

// wait to limit bandwidth usage
static void waitBandwidth (time_t *old)
{
    while (global::remainBand < 0)
    {
        poll(NULL, 0, 10);
        global::now = time(NULL);
        if (*old != global::now)
        {
            *old = global::now;
            cron ();
        }
    }
}

static void transTime(uint t, uint *d, uint *h, uint *m)
{
    uint tm = t / 60;
    uint th = 0;
    uint td = 0;
    if(tm >= 60)
    {
        th = tm / 60;
        tm %= 60;
    }
    if(th >= 24)
    {
        td = th /24;
        th %= 24;
    }
    *d = td;
    *h = th;
    *m = tm;
}

static void printLimitTime(uint t)
{
    uint td, th, tm;
    transTime(t, &td, &th, &tm);
    std::cout << "\e[1;37m[\e[1;32mInfo\e[1;37m]\e[0m Limit Time: ";
    if(td != 0)
        std::cout << td << " Days, ";
    if(th != 0)
        std::cout << th << " Hours, ";
    std::cout << tm << " Minutes." << std::endl;
}

static uint count = 0;

static void getSIGINT(int signo)
{
    std::cout << std::endl;
    closeLevelUp();
}

static void welcome()
{
    std::cout << "####################################" << std::endl;
    std::cout << "#        \e[1;33mLarbin Web Crawler\e[0m        #" << std::endl;
    std::cout << "#                           \e[1;31mv2.6.4\e[0m #" << std::endl;
    std::cout << "####################################" << std::endl;
}

int main (int argc, char *argv[])
{
    welcome();

    // create all the structures
    global glob(argc, argv);
    
    // Start the search
    time_t old = global::now;

    std::cout << "\e[1;37m[\e[1;32mSearch\e[1;37m]\e[0m Starting..." << std::endl;
    if(signal(SIGINT, getSIGINT) == SIG_ERR)
    {
        std::cerr << "\e[1;37m[\e[0;31mError\e[1;37m]\e[0m Can not register \e[1;33mSIGINT\e[0m handle." << std::endl;
        exit(-1);
    }
    // launch the webserver
    if (global::httpPort != 0)
        global::webServerThread = startThread(startWebserver, NULL);
    searchOn();
    if (global::limitTime != 0)
    {
        global::startTime = time(NULL);
        global::limitTimeThread = startThread(pLimitTime, NULL);
        printLimitTime(global::limitTime);
    }

    while (global::searchOn)
    {
        // update time
        global::now = time(NULL);
        if (old != global::now)
        {
            // this block is called every second
            old = global::now;
            if(!cron())
                break;
        }
        stateMain(-count);
        if (global::limitBand != 0)
            waitBandwidth(&old);
        stateMain(1);
        for (uint i = 0; i < global::maxFds; i++)
            global::ansPoll[i] = 0;
        for (uint i = 0; i < global::posPoll; i++)
            global::ansPoll[global::pollfds[i].fd] = global::pollfds[i].revents;
        global::posPoll = 0;
        stateMain(2);
        input();
        stateMain(3);
        sequencer();
        stateMain(4);
        fetchDns();
        stateMain(5);
        fetchOpen();
        stateMain(6);
        checkAll();
        // select
        stateMain(count++);
        poll(global::pollfds, global::posPoll, 10);
        stateMain(7);
    }
    std::cout << "\e[1;37m[\e[1;32mSearch\e[1;37m]\e[0m End." << std::endl;
    while(global::webServerOn)
        sleep(1);
    if (global::httpPort != 0)
        std::cout << "\e[1;37m[\e[1;32mWebserver\e[1;37m]\e[0m End." << std::endl;
    std::cout << "\e[1;37m*** Larbin Close ***\e[0m" << std::endl;
}

// a lot of stats and profiling things
static int cron ()
{
    if (global::URLsDisk->getLength() == 0 && global::URLsDiskWait->getLength() == 0 && debUrl == 0)
        return FALSE;

    // look for timeouts
    checkTimeout();
    // see if we should read again urls in fifowait
    if ((global::now % 300) == 0)
    {
        global::readPriorityWait = global::URLsPriorityWait->getLength();
        global::readWait = global::URLsDiskWait->getLength();
    }
    if ((global::now % 300) == 150)
    {
        global::readPriorityWait = 0;
        global::readWait = 0;
    }

    if (global::limitBand != 0)
    {
        if (global::remainBand > 0)
            global::remainBand = global::limitBand;
        else
            global::remainBand = global::remainBand + global::limitBand;
    }

    if(global::histograms)
        histoHit(pages, answers[success]);

    if (global::printStats)
    {
        if ((global::now & 7) == 0)
        {
            urlsRate = (urls - urlsPrev) >> 3;
            urlsPrev = urls;
            pagesRate = (pages - pagesPrev) >> 3;
            pagesPrev = pages;
            successRate = (answers[success] - successPrev) >> 3;
            successPrev = answers[success];
            siteSeenRate = (siteSeen - siteSeenPrev) >> 3;
            siteSeenPrev = siteSeen;
            siteDNSRate = (siteDNS - siteDNSPrev) >> 3;
            siteDNSPrev = siteDNS;
            if(global::debug)
            {
                readRate = (byte_read - readPrev) >> 3;
                readPrev = byte_read;
                writeRate = (byte_write - writePrev) >> 3;
                writePrev = byte_write;
            }

            uint td, th, tm;
            std::cout << std::endl
                      << ctime(&global::now)
                      << "urls :    " << urls <<             "\t(rate : " << urlsRate <<    ")"
                      << std::endl
                      << "pages :   " << pages <<            "\t(rate : " << pagesRate <<   ")"
                      << std::endl
                      << "success : " << answers[success] << "\t(rate : " << successRate << ")"
                      << std::endl;
            if (global::limitTime != 0)
            {
                std::cout << "Remaining Time: ";
                transTime(global::limitTime - global::now + global::startTime, &td, &th, &tm);
                if(td != 0)
                    std::cout << td << " Days, ";
                if(th != 0)
                    std::cout << th << " Hours, ";
                std::cout << tm << " Minutes." << std::endl;
            }
        }
    }
    return TRUE;
}
