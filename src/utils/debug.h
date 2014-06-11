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

// It's dirty, but I don't care
// it's just for debugging purposes

#ifndef DEBUG_H
#define DEBUG_H

#include "options.h"

////////////////////////////////////////////////
// debug
////////////////////////////////////////////////

// Where are the different threads
extern uint stateMain;
extern uint debug;

// This can be usefull when having very big problem !!!
/* #define stateMain(i) (cerr << "stateMain " << i << "\n") */

#define stateMain(i) (stateMain = i)
#define incDebug() debug++;
#define debug(i) (debug = i)

// Debug new and delete
extern uint sites;
extern uint ipsites;
extern uint debUrl;    // number of urls
extern uint namedUrl;  // urls in namedSites
extern uint missUrl;
extern uint debPars;

#define addsite() sites++
#define addipsite() ipsites++
#define newUrl() debUrl++
#define refUrl() missUrl++
#define delUrl() debUrl--
#define newPars() debPars++
#define delPars() debPars--

#define addNamedUrl() namedUrl++
#define delNamedUrl() namedUrl--

// number of byte read and written
extern unsigned long byte_read;
extern unsigned long byte_write;

#define addRead(i) (byte_read += i)
#define addWrite(i) (byte_write += i)

extern unsigned long readRate;
extern unsigned long readPrev;
extern unsigned long writeRate;
extern unsigned long writePrev;

////////////////////////////////////////////////
// stats
////////////////////////////////////////////////

extern uint siteSeen;
extern uint siteDNS;  // has a DNS entry
extern uint siteRobots;
extern uint robotsOK;
#define siteSeen() siteSeen++
#define siteDNS() siteDNS++
#define siteRobots() siteRobots++
#define robotsOK() robotsOK++
#define robotsOKdec() robotsOK--

extern uint hashUrls;
extern uint urls;
extern uint pages;
extern uint interestingPage;
extern uint interestingSeen;
extern uint interestingSuccess;
extern uint interestingExtension;
extern uint extensionTreated;
extern uint answers[nbAnswers];
#define hashUrls() hashUrls++;
#define urls() urls++
#define pages() pages++
#define interestingPage() interestingPage++
#define interestingSeen() interestingSeen++
#define interestingSuccess() interestingSuccess++
#define interestingExtension() interestingExtension++
#define extensionTreated() extensionTreated++
#define answers(i) answers[i]++

// variables for rates
extern uint urlsRate;
extern uint urlsPrev;
extern uint pagesRate;
extern uint pagesPrev;
extern uint successRate;
extern uint successPrev;
extern uint siteSeenRate;
extern uint siteSeenPrev;
extern uint siteDNSRate;
extern uint siteDNSPrev;

#ifdef CRASH
#define crash(s) (cerr << s << "\n")
#else // CRASH
#define crash(s) ((void) 0)
#endif // CRASH

#endif // DEBUG_H
