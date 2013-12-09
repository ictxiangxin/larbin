#include <iostream>
#include <time.h>
#include <pthread.h>

#include "global.h"
#include "utils/level.h"

void *pLimitTime (void *none)
{
    time_t startTime = time(NULL);
    time_t endTime;
    while(global::searchOn)
    {
        sleep(30);
        endTime = time(NULL);
        if(!global::searchOn)
            break;
        if((uint)(endTime - startTime) >= global::limitTime)
        {
            std::cout << "[Search] Time up." << std::endl;
            closeLevelUp();
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}
