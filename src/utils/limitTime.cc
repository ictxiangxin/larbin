#include <time.h>
#include <pthread.h>
#include <stdio.h>

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
        if((uint)(endTime - startTime) >= global::limitTime)
        {
            printf("[Search] Time up.\n");
            closeLevelUp();
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}
