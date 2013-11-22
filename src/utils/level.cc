#include <stdio.h>

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
                   (global::webServerOn && global::highLevelWebServer) ? "([Webserver] still running)" : "");
            if(!global::highLevelWebServer)
            {
                printf("[Webserver] Closing...\n");
                global::webServerOn = false;
            }
            global::searchOn = false;
            break;
        case 1 :
            if(global::webServerOn)
            {
                printf("[Webserver] Closing...\n");
                global::webServerOn = false;
            }
            break;
        default :
            break;
    }
    global::closeLevel++;
}
