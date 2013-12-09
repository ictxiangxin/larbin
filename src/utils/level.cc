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
