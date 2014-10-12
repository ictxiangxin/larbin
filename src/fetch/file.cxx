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


#include <unistd.h>
#include <iostream>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/text.h"
#include "utils/url.h"
#include "utils/string.h"
#include "utils/vector.h"
#include "fetch/site.h"
#include "fetch/file.h"
#include "fetch/fetch_open.h"
#include "fetch/checker.h"
#include "fetch/save_specific_buffer.h"

#include "utils/debug.h"

#define ANSWER     0
#define HEADERS    1
#define HEADERS30X 2
#define HTML       3
#define SPECIFIC   4

#define LINK 0
#define BASE 1


/***********************************
 * implementation of file
 ***********************************/

file::file (Connexion *conn)
{
    buffer = conn->buffer;
    pos = 0;
    posParse = buffer;
}

file::~file ()
{
}

/***********************************
 * implementation of robots
 ***********************************/

/** Constructor
 */
robots::robots (NamedSite *server, Connexion *conn) : file(conn)
{
    newPars();
    this->server = server;
    answerCode = false;
    isRobots = true;
}

/** Destructor
 */
robots::~robots ()
{
    delPars();
    // server is not deleted on purpose
    // it belongs to someone else
}

/** we get some more chars of this file
 */
int robots::endInput ()
{
    return 0;
}

/** input and parse headers
 */
int robots::inputHeaders (int size)
{
    pos += size;
    if (!answerCode && pos > 12)
    {
        if (buffer[9] == '2')
            answerCode = true;
        else
        {
            errno = err40X;
            return 1;
        }
    }
    if (pos > maxRobotsSize)
    {
        // no more input, forget the end of this file
        errno = tooBig;
        return 1;
    }
    else
        return 0;
}

/** parse the robots.txt
 */
void robots::parse (bool isError)
{
    if (answerCode && parseHeaders())
    {
        siteRobots();
        buffer[pos] = 0;
        if (isError)
        {
            // The file could be incomplete, delete last token
            // We could have Disallow / instead of Disallow /blabla
            for (uint i = pos - 1; i > 0 && !isspace(buffer[i]); i--)
                buffer[i] = ' ';
        }
        parseRobots();
    }
}

/** test http headers
 * return true if OK, false otherwise
 */
bool robots::parseHeaders ()
{
    for(posParse = buffer + 9; posParse[3] != '\0'; posParse++)
        if (
            (
                posParse[0] == '\n'
                && (
                    posParse[1] == '\n'
                    || posParse[2] == '\n'
                )
            )
            || (
                posParse[0] == '\r'
                && (
                    posParse[1] == '\r'
                    || posParse[2] == '\r'
                )
            )
        )
            return true;
    return false;
}

/** try to understand the file
 */
void robots::parseRobots ()
{
    robotsOK();
    bool goodfile = true;
    server->forbidden.recycle();
    uint items = 0; // size of server->forbidden
    // state
    // 0 : not concerned
    // 1 : weakly concerned
    // 2 : strongly concerned
    int state = 1;
    char *tok = nextToken(&posParse, ':');
    while (tok != NULL)
    {
        if (!strcasecmp(tok, "useragent") || !strcasecmp(tok, "user-agent"))
        {
            if (state == 2)
                return; // end of strong concern record => the end for us
            else
            {
                state = 0;
                // what is the new state ?
                tok = nextToken(&posParse, ':');
                while (tok != NULL && strcasecmp(tok, "useragent") && strcasecmp(tok, "user-agent") && strcasecmp(tok, "disallow"))
                {
                    if (caseContain(tok, global::userAgent))
                        state = 2;
                    else if (state == 0 && !strcmp(tok, "*"))
                        state = 1;
                    tok = nextToken(&posParse, ':');
                }
            }
            if (state)
            {
                // delete old forbidden : we've got a better record than older ones
                server->forbidden.recycle();
                items = 0;
            }
            else
            {
                // forget this record
                while (tok != NULL && strcasecmp(tok, "useragent") && strcasecmp(tok, "user-agent"))
                    tok = nextToken(&posParse, ':');
            }
        }
        else if (!strcasecmp(tok, "disallow"))
        {
            tok = nextToken(&posParse, ':');
            while (tok != NULL && strcasecmp(tok, "useragent") && strcasecmp(tok, "user-agent") && strcasecmp(tok, "disallow"))
            {
                // add nextToken to forbidden
                if (items++ < maxRobotsItem)
                {
                    // make this token a good token
                    if (tok[0] == '*') // * is not correct, / disallows everything
                        tok[0] = '/';
                    else if (tok[0] != '/')
                    {
                        tok--;
                        tok[0] = '/';
                    }
                    if (fileNormalize(tok))
                        server->forbidden.addElement(newString(tok));
                }
                tok = nextToken(&posParse, ':');
            }
        }
        else
        {
            if (global::printStats)
                if (goodfile)
                {
                    robotsOKdec();
                    goodfile = false;
                }
            tok = nextToken(&posParse, ':');
        }
    }
}


/*************************************
 * implementation of html
 *************************************/

#define _newSpec() \
            do { \
                if (state==SPECIFIC) \
                    newSpec(); \
            } while(0)

#define _destructSpec() \
            do { \
                if (state==SPECIFIC) \
                    destructSpec(); \
            } while(0)

#define _endOfInput() \
            do { \
                if (state==SPECIFIC) \
                    return endOfInput(); \
            } while(0)

#define _getContent() \
            do { \
                if (state==SPECIFIC) \
                    return getContent(); \
                else \
                    return contentStart; \
            } while(0)

#define _getSize() \
            do { \
                if (state==SPECIFIC) \
                    return getSize(); \
                else \
                    return (buffer + pos - contentStart); \
            } while(0)

#define notCgiChar(c) (global::getCGI || (c!='?' && c!='=' && c!='*'))

/** Constructor
 */
html::html (url *here, Connexion *conn) : file(conn)
{
    newPars();
    this->here = here;
    base = here->giveBase();
    state = ANSWER;
    isInteresting = false;
    constrSpec();
    pages();
    isRobots = false;
}

/** Destructor
 */
html::~html ()
{
    _destructSpec();
    delPars();
    delete here;
    delete base;
}

/* get the content of the page */
char *html::getPage ()
{
    _getContent();
}

int html::getLength ()
{
    _getSize();
}

/* manage a new url : verify and send it */
void html::manageUrl (url *nouv, bool isRedir)
{
    if (
        nouv->isValid()
        && filter1(nouv->getHost(), nouv->getFile())
        && (
            global::externalLinks
            || isRedir
            || !strcmp(nouv->getHost(), this->here->getHost())
        )
    )
    {
        // The extension is not stupid (gz, pdf...)
#ifdef LINKS_INFO
        links.addElement(nouv->giveUrl());
#endif // LINKS_INFO
        if (nouv->initOK(here))
            check(nouv);
        else
        {
            // this url is forbidden for errno reason (set by initOK)
            answers(errno);
            delete nouv;
        }
    }
    else
        // The extension is stupid
        delete nouv;
}

/**********************************************/
/* This part manages command line and headers */
/**********************************************/

/** a string is arriving, treat it only up to the end of headers
 * return 0 usually, 1 if no more input and set errno accordingly
 */
int html::inputHeaders (int size)
{
    pos += size;
    buffer[pos] = 0;
    char *posn;
    while (posParse < buffer + pos)
    {
        switch (state)
        {
        case ANSWER:
            posn = strchr(posParse, '\n');
            if (posn != NULL)
            {
                posParse = posn;
                if (parseCmdline ())
                    return 1;
                area = ++posParse;
            }
            else
                return 0;
            break;
        case HEADERS:
        case HEADERS30X:
            posn = strchr(posParse, '\n');
            if (posn != NULL)
            {
                posParse = posn;
                int tmp;
                if (state == HEADERS)
                    tmp = parseHeader();
                else
                    tmp = parseHeader30X();
                if (tmp)
                    return 1;
                area = ++posParse;
            }
            else
                return 0;
            break;
        case SPECIFIC:
            return pipeSpec();
        default:
            return 0;
        }
    }
    return 0;
}

/** parse the answer code line */
int html::parseCmdline ()
{
    if (posParse - buffer >= 12)
    {
        switch (buffer[9])
        {
        case '2':
            state = HEADERS;
            break;
        case '3':
            state = HEADERS30X;
            break;
        default:
            errno = err40X;
            return 1;
        }
    }
    else
    {
        errno = earlyStop;
        return 1;
    }
    return 0;
}

/** parse a line of header
 * @return 0 if OK, 1 if we don't want to read the file
 */
int html::parseHeader ()
{
    if (posParse - area < 2)
    {
        // end of http headers
#ifndef FOLLOW_LINKS
        state = SPECIFIC;
#else
        if (global::specificSearch)
        {
            if (isInteresting)
                state = SPECIFIC;
            else
                state = HTML;
        }
        else
            state = HTML;
#endif
        contentStart = posParse + 1;
        *(posParse - 1) = 0;
        _newSpec();
    }
    else
    {
        *posParse = 0;
        if (global::useCookies)
            here->addCookie(area);
        *posParse = '\n';
        if (verifType () || verifLength())
            return 1;
    }
    return 0;
}

int html::verifType ()
{
    if (startWithIgnoreCase((char*)"content-type: ", area))
        // Let's read the type of this doc
        if (!startWithIgnoreCase((char*)"text/html", area + 14))
        {
            if (global::specificSearch && (extIndex = matchContentType(area + 14)) != -1)
            {
                interestingSeen();
                isInteresting = true;
            }
            else
            {
                if (global::anyType)
                    return 0;
                if (global::getImage && startWithIgnoreCase("image", area + 14))
                    return 0;
                else
                {
                    errno = badType;
                    return 1;
                }
            }
        }
    return 0;
}

/** function called by parseHeader
 * parse content-length
 * return 1 (and set errno) if too long file, 0 otherwise
 */
int html::verifLength ()
{
    if (!global::specificSearch && startWithIgnoreCase((char*)"content-length: ", area))
    {
        int len = 0;
        char *p = area + 16;
        while (*p >= '0' && *p <= '9')
        {
            len = len * 10 + *p - '0';
            p++;
        }
        if (len > maxPageSize)
        {
            errno = tooBig;
            return 1;
        }
    }
    return 0;
}

/** parse a line of header (ans 30X) => just look for location
 * @return 0 if OK, 1 if we don't want to read the file
 */
int html::parseHeader30X ()
{
    if (posParse - area < 2)
    {
        // end of http headers without location => err40X
        errno = err40X;
        return 1;
    }
    else
    {
        if (startWithIgnoreCase((char*)"location: ", area))
        {
            int i=10;
            while (area[i]!=' ' && area[i]!='\n' && area[i]!='\r' && notCgiChar(area[i]))
                i++;
            if (notCgiChar(area[i]))
            {
                area[i] = 0; // end of url
                // read the location (do not decrease depth)
                url *nouv = new url(area+10, here->getDepth(), base);
#ifdef URL_TAGS
                nouv->tag = here->tag;
#endif // URL_TAGS
                manageUrl(nouv, true);
                // we do not need more headers
            }
            errno = err30X;
            return 1;
        }
    }
    return 0;
}

/*********************************************/
/* This part manages the content of the file */
/*********************************************/

/** file download is complete, parse the file (headers already done)
 * return 0 usually, 1 if there was an error
 */
int html::endInput ()
{
    if (state <= HEADERS)
    {
        errno = earlyStop;
        return 1;
    }
    if (state == HEADERS30X)
    {
        errno = err40X;
        return 1;
    }
    if (global::pageNoDuplicate)
        if (!global::hDuplicate->testSet(posParse))
        {
            errno = duplicate;
            return 1;
        }
    buffer[pos] = 0;
    _endOfInput();
    // now parse the html
    parseHtml();
    return 0;
}

/* parse an html page */
void html::parseHtml ()
{
    while ((posParse = strchr(posParse, '<')) != NULL)
    {
        if (posParse[1] == '!')
        {
            if (posParse[2] == '-' && posParse[3] == '-')
            {
                posParse += 4;
                parseComment();
            }
            else
                // nothing...
                posParse += 2;
        }
        else
        {
            posParse++;
            parseTag();
        }
    }
}

/* skip a comment */
void html::parseComment()
{
    while ((posParse=strchr(posParse, '-')) != NULL)
    {
        if (posParse[1] == '-' && posParse[2] == '>')
        {
            posParse += 3;
            return;
        }
        else
            posParse++;
    }
    posParse = buffer+pos;
}

/* macros used by the following functions */
#define skipSpace() \
            do { \
                while (*posParse == ' ' || *posParse == '\n' || *posParse == '\r' || *posParse == '\t') \
                    posParse++; \
            } while(0)

#define skipText() \
            do { \
                while (*posParse != ' ' && *posParse != '\n' && *posParse != '>'  && *posParse != '\r' && *posParse != '\t' && *posParse != 0) \
                    posParse++; \
            } while(0)

#define nextWord() \
            do { \
                skipText(); \
                skipSpace(); \
            } while(0)
#define thisCharIs(i, c) (c == (posParse[i] | 32))
#define ISTAG(t, p, a, i) \
            do { \
                if (t) \
                { \
                    param = p; \
                    action = a; \
                    posParse += i; \
                } \
                else \
                { \
                    posParse++; \
                    return; \
                } \
            } while(0)

/** Try to understand this tag */
void html::parseTag ()
{
    skipSpace();
    const char *param = NULL; // what parameter are we looking for
    int action = -1;
    // read the name of the tag
    if (thisCharIs(0, 'a'))
    {
        // a href
        param = "href";
        action = LINK;
        posParse++;
    }
    else if (thisCharIs(0, 'l'))
    {
        ISTAG(thisCharIs(1, 'i') && thisCharIs(2, 'n') && thisCharIs(3, 'k'), "href", LINK, 4);
    }
    else if (thisCharIs(0, 'b'))
    {
        ISTAG(thisCharIs(1, 'a') && thisCharIs(2, 's') && thisCharIs(3, 'e'), "href", BASE, 4);
    }
    else if (thisCharIs(0, 'f'))
    {
        ISTAG(thisCharIs(1, 'r') && thisCharIs(2, 'a') && thisCharIs(3, 'm') && thisCharIs(4, 'e'), "src", LINK, 5);
    }
    else if (global::getImage && thisCharIs(0, 'i'))
    {
        ISTAG(thisCharIs(1, 'm') && thisCharIs(2, 'g'), "src", LINK, 3);
    }
    else
        return;
    // now find the parameter
    assert(param != NULL);
    skipSpace();
    for (;;)
    {
        int i = 0;
        while (param[i]!= 0 && thisCharIs(i, param[i]))
            i++;
        posParse += i;
        skipSpace();
        if (posParse[i] == '>' || posParse[i] == 0)
            return;
        if (param[i] == 0)
        {
            parseContent(action);
            return;
        }
        else
            // not the good parameter
            nextWord();
    }
}

/** read the content of an interesting tag */
void html::parseContent (int action)
{
    posParse++;
    while (*posParse==' ' || *posParse=='=')
        posParse++;
    if (*posParse=='\"' || *posParse=='\'')
        posParse++;
    area = posParse;
    char *endItem = area + maxUrlSize;
    if (endItem > buffer + pos)
        endItem = buffer + pos;
    while (
        posParse < endItem
        && *posParse != '\"'
        && *posParse != '\''
        && *posParse != '\n'
        && *posParse != ' '
        && *posParse != '>'
        && *posParse != '\r'
        && *posParse != '\t'
        && notCgiChar(*posParse)
    )
    {
        if (*posParse == '\\')
            *posParse = '/';    // Bye Bye DOS !
        posParse++;
    }
    if (posParse == buffer + pos) // end of file => content may be truncated => forget it
        return;
    else if (posParse < endItem && notCgiChar(*posParse))
    {
        // compute this url (not too long and not cgi)
        char oldchar = *posParse;
        *posParse = 0;
        switch (action)
        {
        case LINK:
            // try to understand this new link
            manageUrl(new url(area, here->getDepth() - 1, base), false);
            break;
        case BASE:
        {
            // This page has a BASE HREF tag
            uint end = posParse - area - 1;
            if(posParse == area)
                break;
            while (end > 7 && area[end] != '/') // 7 because http://
                end--;
            if (end > 7) // this base looks good
            {
                end++;
                char tmp = area[end];
                area[end] = 0;
                url *tmpbase = new url(area, 0, (url *) NULL);
                area[end] = tmp;
                delete base;
                if (tmpbase->isValid())
                    base = tmpbase;
                else
                {
                    delete tmpbase;
                    base = NULL;
                }
            }
            break;
        }
        default:
            assert(false);
        }
        *posParse = oldchar;
    }
    posParse++;
}
