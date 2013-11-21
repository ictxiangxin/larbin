#include <time.h>
#include <pthread.h>

#include "global.h"

void *pLimitTime (void *none)
{
    time_t startTime = time(NULL);
    time_t endTime;
    while(true)
    {
        sleep(60);
        endTime = time(NULL);
        if((uint)(endTime - startTime) >= global::limitTime)
        {
            global::timeOut = true;
            pthread_exit(NULL);
        }
    }
}
