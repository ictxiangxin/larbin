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

static void cron ();

///////////////////////////////////////////////////////////
#ifdef PROF
static bool stop = false;
static void handler (int i)
{
    stop = true;
}
#endif // PROF

// wait to limit bandwidth usage
#ifdef MAXBANDWIDTH
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
#else
#define waitBandwidth(x) ((void) 0)
#endif // MAXBANDWIDTH

static void printLimitTime(uint t)
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
        td %= 24;
    }
    printf("Limit Time: ");
    if(td != 0)
        printf("%d Days, ", td);
    if(th != 0)
        printf("%d Hours, ", th);
    printf("%d Minutes.\n", tm);
}

#ifndef NDEBUG
static uint count = 0;
#endif // NDEBUG

static void userEnd(int signo)
{
    printf("\n");
    closeLevelUp();
}

static void welcome()
{
    printf("####################################\n");
    printf("#        Larbin Web Crawler        #\n");
    printf("#                           v2.6.4 #\n");
    printf("####################################\n");
}

///////////////////////////////////////////////////////////
// If this thread terminates, the whole program exits
int main (int argc, char *argv[])
{
    welcome();
    // create all the structures
    global glob(argc, argv);
#ifdef PROF
    signal (2, handler);
#endif // PROF

#ifndef NOWEBSERVER
    // launch the webserver if needeed
    if (global::httpPort != 0)
        startThread(startWebserver, NULL);
#endif // NOWEBSERVER
    printf("User Agent: %s\n", global::userAgent);
    if (global::limitTime != 0)
    {
        startThread(pLimitTime, NULL);
        printLimitTime(global::limitTime);
    }
    // Start the search
    time_t old = global::now;

    printf("[Search] Starting...\n");
    if(signal(SIGINT, userEnd) == SIG_ERR)
    {
        std::cerr << "Can not register" << std::endl;
        exit(0);
    }
    searchOn();
    while (global::searchOn)
    {
        // update time
        global::now = time(NULL);
        if (old != global::now)
        {
            // this block is called every second
            old = global::now;
            cron();
        }
        stateMain(-count);
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
    printf("[Search] End.\n");
    while(global::webServerOn)
        sleep(1);
    printf("*** Larbin Close ***\n");
}

// a lot of stats and profiling things
static void cron ()
{
    // shall we stop
#ifdef PROF
    if (stop)
        exit(2);
#endif // PROF
#ifdef EXIT_AT_END
    if (global::URLsDisk->getLength() == 0 && global::URLsDiskWait->getLength() == 0 && debUrl == 0)
        exit(0);
#endif // EXIT_AT_END

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

#ifdef MAXBANDWIDTH
    // give bandwidth
    if (global::remainBand > 0)
        global::remainBand = MAXBANDWIDTH;
    else
        global::remainBand = global::remainBand + MAXBANDWIDTH;
#endif // MAXBANDWIDTH

#ifndef NOSTATS
    histoHit(pages, answers[success]);

    if ((global::now & 7) == 0)
    {
        urlsRate = (urls - urlsPrev) >> 3; urlsPrev = urls;
        pagesRate = (pages - pagesPrev) >> 3; pagesPrev = pages;
        successRate = (answers[success] - successPrev) >> 3;
        successPrev = answers[success];
        siteSeenRate = (siteSeen - siteSeenPrev) >> 3; siteSeenPrev = siteSeen;
        siteDNSRate = (siteDNS - siteDNSPrev) >> 3; siteDNSPrev = siteDNS;
#ifndef NDEBUG
        readRate = (byte_read - readPrev) >> 3; readPrev = byte_read;
        writeRate = (byte_write - writePrev) >> 3; writePrev = byte_write;
#endif // NDEBUG

#ifdef STATS
        printf("\n%surls : %d  (rate : %d)\npages : %d  (rate : %d)\nsuccess : %d  (rate : %d)\n",
               ctime(&global::now), urls, urlsRate, pages, pagesRate,
               answers[success], successRate);
#endif // STATS
    }
#endif // NOSTATS
}
