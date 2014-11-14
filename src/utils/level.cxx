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
#include <pthread.h>

#include "global.h"

void webServerOn()
{
    global::webServerOn = true;
}

void webServerOff()
{
    global::webServerOn = false;
}

void searchOn()
{
    global::searchOn = true;
}

void searchOff()
{
    global::searchOn = false;
}

void closeLevelUp(int l)
{
    switch(global::closeLevel)
    {
    case LEVEL_SEARCH :
        if (global::searchOn)
        {
            std::cout << "["GREEN_MSG("Search")"] Closing... ";
            if (global::webServerOn && global::highLevelWebServer)
                std::cout << "(["GREEN_MSG("Webserver")"] still running)";
            std::cout << std::endl;
            if (global::webServerOn)
                if(!global::highLevelWebServer)
                {
                    std::cout << "["GREEN_MSG("Webserver")"] Closing..." << std::endl;
                    global::webServerOn = false;
                }
            global::searchOn = false;
        }
        break;
    case LEVEL_WEBSERVER :
        if(global::webServerOn)
        {
            std::cout << "["GREEN_MSG("Webserver]")"] Closing..." << std::endl;
            global::webServerOn = false;
        }
        break;
    default :
        break;
    }
    if (l < 0 || l >= global::closeLevel)
        global::closeLevel++;
}
