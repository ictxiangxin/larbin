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

void closeLevelUp()
{
    switch(global::closeLevel)
    {
        case 0 :
            std::cout << "[Search] Closing... ";
#ifndef NOWEBSERVER
            if (global::webServerOn && global::highLevelWebServer)
                std::cout << "([Webserver] still running)";
#endif // NOWEBSERVER
            std::cout << std::endl;
#ifndef NOWEBSERVER
            if(!global::highLevelWebServer)
            {
                std::cout << "[Webserver] Closing..." << std::endl;
                global::webServerOn = false;
                pthread_cancel(global::webServerThread);
            }
#endif // NOWEBSERVER
            global::searchOn = false;
            pthread_cancel(global::limitTimeThread);
            break;
        case 1 :
#ifndef NOWEBSERVER
            if(global::webServerOn)
            {
                std::cout << "[Webserver] Closing..." << std::endl;
                global::webServerOn = false;
                pthread_cancel(global::webServerThread);
            }
#endif // NOWEBSERVER
            break;
        default :
            break;
    }
    global::closeLevel++;
}
