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

#ifndef URL_H
#define URL_H

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "types.h"

bool fileNormalize (char *file);

class url
{
    private:
        char *host;
        char *punycode;
        char *file;
        uint16_t port; // the order of variables is important for physical size
        int depth;
        /* parse the url */
        void parse (char *s);
        /** parse a file with base */
        void parseWithBase (char *u, url *base);
        /* normalize file name */
        bool normalize ();
        /* Does this url starts with a protocol name */
        bool isProtocol (char *s);
        /* constructor used by giveBase */
        url (char *host, uint port, char *file);

    public:
        /* Constructor : Parses an url (u is deleted) */
        url (char *u, int depth, url *base);

        /* constructor used by input */
        url (char *line, int depth);

        /* Constructor : read the url from a file (cf serialize) */
        url (char *line);

        /* Destructor */
        ~url ();

        /* inet addr (once calculated) */
        struct in_addr addr;

        /* Is it a valid url ? */
        bool isValid ();

        /* print an URL */
        void print ();

        /* get punycode host */
        char *getPunycode();

        /* return the host */
        inline char *getHost () { return host; }

        /* return the port */
        inline uint getPort () { return port; }

        /* return the file */
        inline char *getFile () { return file; }

        /** Depth in the Site */
        inline int getDepth () { return depth; }

        /* Set depth to max if we are at an entry point in the site
         * try to find the ip addr
         * answer false if forbidden by robots.txt, true otherwise */
        bool initOK (url *from);

        /** return the base of the url
         * give means that you have to delete the string yourself
         */
        url *giveBase ();

        /** return a char * representation of the url
         * give means that you have to delete the string yourself
         */
        char *giveUrl ();

        /** write the url in a buffer
         * buf must be at least of size maxUrlSize
         * returns the size of what has been written (not including '\0')
         */
        int writeUrl (char *buf);

        /* serialize the url for the Persistent Fifo */
        char *serialize ();

        /* very thread unsafe serialisation in a static buffer */
        char *getUrl();

        /* return a hashcode for the host of this url */
        uint hostHashCode ();

        /* return a hashcode for this url */
        uint hashCode ();

#ifdef URL_TAGS
        /* tag associated to this url */
        uint tag;
#endif // URL_TAGS

        /* cookies associated with this page */
        char *cookie;
        void addCookie(char *header);
};

#endif // URL_H
