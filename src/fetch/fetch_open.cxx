#include <iostream>
#include <errno.h>
#include <sys/types.h>

#include <adns.h>

#include "options.h"

#include "global.h"
#include "utils/fifo.h"
#include "utils/debug.h"
#include "fetch/site.h"

/* Opens sockets
 * Never block (only opens sockets on already known sites)
 * work inside the main thread
 */
void fetchOpen ()
{
    static time_t next_call = 0;
    if (global::now < next_call) // too early to come back
        return;
    bool cont = true;
    while (cont && global::freeConns->isNonEmpty())
    {
        IPSite *s = global::okSites->tryGet();
        if (s == NULL)
            cont = false;
        else
        {
            next_call = s->fetch();
            cont = (next_call == 0);
        }
    }
}

/* Opens sockets
 * this function perform dns calls, using adns
 */
void fetchDns ()
{
    // Submit queries
    // try to avoid too many dns calls
    while (global::nbDnsCalls < global::dnsConn && global::freeConns->isNonEmpty() && global::IPUrl < maxIPUrls)
    {
        NamedSite *site = global::dnsSites->tryGet();
        if (site == NULL)
            break;
        else
            site->newQuery();
    }
    // Read available answers
    while (global::nbDnsCalls && global::freeConns->isNonEmpty())
    {
        NamedSite *site;
        adns_query quer = NULL;
        adns_answer *ans;
        int res = adns_check(global::ads, &quer, &ans, (void**)&site);
        if (res == ESRCH || res == EAGAIN) // No more query or no more answers
            break;
        global::nbDnsCalls--;
        site->dnsAns(ans);
        free(ans); // ans has been allocated with malloc
    }
}
