#include <stdio.h>
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
            printf("[Search] Closing... %s\n",
#ifndef NOWEBSERVER
                   (global::webServerOn && global::highLevelWebServer) ? "([Webserver] still running)" :
#endif // NOWEBSERVER
                   "");
#ifndef NOWEBSERVER
            if(!global::highLevelWebServer)
            {
                printf("[Webserver] Closing...\n");
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
                printf("[Webserver] Closing...\n");
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
