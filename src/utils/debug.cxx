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

#include "options.h"

#include "types.h"

uint stateMain;          // where is the main loop
uint debug = 0;

uint sites = 0;          // Number of occupied sites
uint ipsites = 0;        // Number of occupied ip sites
uint debUrl = 0;         // How many urls in ram
uint namedUrl = 0;
uint missUrl = 0;        // Number of urls reput in ram
uint debPars = 0;        // How many parsers are there

unsigned long byte_read = 0;
unsigned long byte_write = 0;

unsigned long readRate = 0;
unsigned long readPrev = 0;
unsigned long writeRate = 0;
unsigned long writePrev = 0;

uint siteSeen = 0;
uint siteDNS = 0;         // has a DNS entry
uint siteRobots = 0;
uint robotsOK = 0;

uint hashUrls = 0;
uint urls = 0;
uint pages = 0;
uint interestingPage = 0;
uint interestingSeen = 0;
uint interestingExtension = 0;
uint extensionTreated = 0;
uint answers[nbAnswers] = {0,0,0,0,0,0,0,0,0};

// variables for rates
uint urlsRate = 0;
uint urlsPrev = 0;
uint pagesRate = 0;
uint pagesPrev = 0;
uint successRate = 0;
uint successPrev = 0;
uint siteSeenRate = 0;
uint siteSeenPrev = 0;
uint siteDNSRate = 0;
uint siteDNSPrev = 0;
