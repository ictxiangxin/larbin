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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/url.h"
#include "utils/text.h"
#include "utils/string.h"
#include "utils/connection.h"
#include "fetch/site.h"
#include "fetch/file.h"
#include "io/output.h"

#include "utils/debug.h"

static void pipeRead (Connexion *conn);
static void pipeWrite (Connexion *conn);
static void endOfFile (Connexion *conn);


/*
 * Check timeout
 */
void checkTimeout ()
{
    for (uint i = 0; i < global::nb_conn; i++)
    {
        Connexion *conn = global::connexions + i;
        if (conn->state != emptyC)
        {
            if (conn->timeout > 0)
            {
                if (conn->timeout > timeoutPage)
                    conn->timeout = timeoutPage;
                else
                    conn->timeout--;
            }
            else
            {
                // This server doesn't answer (time out)
                conn->err = timeout;
                endOfFile(conn);
            }
        }
    }
}

/*
 * read all data available
 * fill fd_set for next select
 * give back max fds
 */
void checkAll ()
{
    // read and write what can be
    for (uint i = 0; i < global::nb_conn; i++)
    {
        Connexion *conn = global::connexions + i;
        switch (conn->state)
        {
        case connectingC :
        case writeC :
            if (global::ansPoll[conn->socket])
                pipeWrite(conn); // trying to finish the connection
            break;
        case openC:
            if (global::ansPoll[conn->socket])
                pipeRead(conn); // The socket is open, let's try to read it
            break;
        }
    }

    // update fd_set for the next select
    for (uint i = 0; i < global::nb_conn; i++)
    {
        int n = (global::connexions + i)->socket;
        switch ((global::connexions + i)->state)
        {
        case connectingC:
        case writeC:
            setPoll(n, POLLOUT);
            break;
        case openC:
            setPoll(n, POLLIN);
            break;
        }
    }
}

/*
 * The socket is finally open !
 * Make sure it's all right, and write the request
 */
static void pipeWrite (Connexion *conn)
{
    int res = 0;
    int wrtn, len;
    socklen_t size = sizeof(int);
    switch (conn->state)
    {
    case connectingC:
        // not connected yet
        getsockopt(conn->socket, SOL_SOCKET, SO_ERROR, &res, &size);
        if (res)
        {
            // Unable to connect
            conn->err = noConnection;
            endOfFile(conn);
            return;
        }
        // Connection succesfull
        conn->state = writeC;
    // no break
    case writeC:
        // writing the first string
        len = strlen(conn->request.getString());
        wrtn = write(conn->socket, conn->request.getString()+conn->pos,
                     len - conn->pos);
        if (wrtn >= 0)
        {
            addWrite(wrtn);
            if (global::limitBand)
                global::remainBand -= wrtn;
            conn->pos += wrtn;
            if (conn->pos < len)
            {
                // Some chars of this string are not written yet
                return;
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EINTR || errno == ENOTCONN)
            {
                // little error, come back soon
                return;
            }
            else
            {
                // unrecoverable error, forget it
                conn->err = earlyStop;
                endOfFile(conn);
                return;
            }
        }
        // All the request has been written
        conn->state = openC;
    }
}

/*
 * Is there something to read on this socket
 * (which is open)
 */
static void pipeRead (Connexion *conn)
{
    int p = conn->parser->pos;
    int size = read (conn->socket, conn->buffer+p, maxPageSize-p-1);
    switch (size)
    {
    case 0:
        // End of file
        if (conn->parser->endInput())
            conn->err = (FetchError) errno;
        endOfFile(conn);
        break;
    case -1:
        switch (errno)
        {
        case EAGAIN:
            // Nothing to read now, we'll try again later
            break;
        default:
            // Error : let's forget this page
            conn->err = earlyStop;
            endOfFile(conn);
            break;
        }
        break;
    default:
        // Something has been read
        conn->timeout += size / timeoutIncr;
        addRead(size);
        if (global::limitBand != 0)
            global::remainBand -= size;
        if (conn->parser->inputHeaders(size) == 0)
        {
            // nothing special
            if (conn->parser->pos >= maxPageSize-1)
            {
                // We've read enough...
                conn->err = tooBig;
                endOfFile(conn);
            }
        }
        else
        {
            // The parser does not want any more input (errno explains why)
            conn->err = (FetchError) errno;
            endOfFile(conn);
        }
        break;
    }
}


/* What are we doing when it's over with one file ? */

#ifdef THREAD_OUTPUT
#define manageHtml() global::userConns->put(conn)
#else // THREAD_OUTPUT
#define manageHtml() \
    endOfLoad((html *)conn->parser, conn->err); \
    conn->recycle(); \
    global::freeConns->put(conn)
#endif // THREAD_OUTPUT

static void endOfFile (Connexion *conn)
{
    conn->state = emptyC;
    close(conn->socket);
    if (conn->parser->isRobots)
    {
        // That was a robots.txt
        robots *r = ((robots *) conn->parser);
        r->parse(conn->err != success);
        r->server->robotsResult(conn->err);
        conn->recycle();
        global::freeConns->put(conn);
    }
    else
    {
        // that was an html page
        manageHtml();
    }
}
