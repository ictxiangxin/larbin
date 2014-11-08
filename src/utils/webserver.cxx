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

#include <string.h>
#include <unistd.h>
#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "options.h"

#include "global.h"

#include "fetch/sequencer.h"

#include "utils/text.h"
#include "utils/connection.h"
#include "utils/debug.h"
#include "utils/histogram.h"
#include "utils/level.h"

#include "io/user_output.h"

#define HTTP(X) ecrire(fds, X)

static char *readRequest (int fds);
static void manageAns (int fds, char *req);

static int totalduree;
static time_t startTime;
static char *startDate;

// a buffer used for various things (read request, write urls...)
static char buf[BUF_SIZE];

void *startWebserver (void *none)
{
    // bind the socket
    int fds;
    int nAllowReuse = 1;
    struct sockaddr_in addr;
    startTime = global::now;
    startDate = newString(ctime(&startTime));
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(global::httpPort);
    if (
        (fds = socket(AF_INET, SOCK_STREAM, 0)) == -1
        || setsockopt(fds, SOL_SOCKET, SO_REUSEADDR, (char*)&nAllowReuse, sizeof(nAllowReuse))
        || bind(fds, (struct sockaddr *) &addr, sizeof(addr)) != 0
        || listen(fds, 4) != 0)
    {
        std::cerr << "["RED_MSG("Error")"] Unable to get the socket for the webserver (" << global::httpPort << ") : " << strerror(errno) << std::endl;
        webServerOff();
        pthread_exit(NULL);
    }
    webServerOn();
    // answer requests
    while (global::webServerOn)
    {
        struct sockaddr_in addrc;
        int fdc;
        socklen_t len = sizeof(addr);
        fdc = accept(fds, (struct sockaddr *) &addrc, &len);
        if (fdc == -1)
            std::cerr << "["RED_MSG("Error")"] Trouble with web server..." << std::endl;
        else
        {
            manageAns(fdc, readRequest(fdc));
            close(fdc);
        }
    }
    pthread_exit(NULL);
}


////////////////////////////////////////////////////////////
// write answer

static void writeHeader(int fds)
{
    ecrire(fds, (char*)"HTTP/1.0 200 OK\r\nServer: Larbin\r\nContent-type: text/html\r\n\r\n");
    HTTP("<html>\n");
    HTTP("<head>\n");
    HTTP("<link rel=\"stylesheet\" href=\"http://cdn.bootcss.com/bootstrap/3.3.0/css/bootstrap.min.css\">\n");
    HTTP("<script src=\"http://cdn.bootcss.com/jquery/1.11.1/jquery.min.js\"></script>\n");
    HTTP("<script src=\"http://cdn.bootcss.com/bootstrap/3.3.0/js/bootstrap.min.js\"></script>\n");
    HTTP("<title>");
    HTTP(global::userAgent);
    HTTP(" real time statistic</title>\n");
    HTTP("</head>\n");
    HTTP("<body>\n");
    HTTP("<center><h1>Larbin is");
    if (!global::searchOn)
        HTTP(" end ");
    else
        HTTP(" up and running ");
    HTTP(" <small>v2.6.5</small></h1></center>\n");
    HTTP("<div class=\"container\">\n");
    HTTP("<div class=\"row\">\n");
    HTTP("<nav class=\"navbar navbar-inverse\" role=\"navigation\">\n");
    HTTP("<div class=\"container-fluid\">\n");
    HTTP("<div class=\"navbar-header\">\n");
    HTTP("<a class=\"navbar-brand\">Larbin</a>\n");
    HTTP("</div>\n");
    HTTP("<ul class=\"nav navbar-nav\">\n");
    HTTP("<li><a href=\"/stats.html\">Stats</a></li>\n");
    HTTP("<li><a href=\"/histograms.html\">Histograms</a></li>\n");
    HTTP("<li><a href=\"/urls.html\">URLs</a></li>\n");
    HTTP("<li><a href=\"/dns.html\">DNS</a></li>\n");
    HTTP("<li><a href=\"/system.html\">System</a></li>\n");
    HTTP("<li><a href=\"/output.html\">Output</a></li>\n");
    HTTP("</ul>\n");
    HTTP("<ul class=\"nav navbar-nav navbar-right\">\n");
    HTTP("<li><a>Update ");
    int duree = global::now - startTime;
    totalduree = duree;
    if (duree > 86400)
    {
        ecrireInt(fds, duree / 86400);
        duree %= 86400;
        HTTP(" Day(s) ");
    }
    ecrireInt(fds, duree/3600);
    duree %= 3600;
    HTTP(":");
    ecrireInt2(fds, duree/60);
    duree %= 60;
    HTTP(":");
    ecrireInt2(fds, duree);
    HTTP("</a></li>\n");
    HTTP("</ul>\n");
    HTTP("</div>\n");
    HTTP("</nav>\n");
    HTTP("</div>\n");
}

static void writeFooter(int fds)
{
    // end of page and kill the connexion
    HTTP("</div>\n</body>\n</html>");
    shutdown(fds, 2);
}

/* write stats information (if any) */

// specific part
static void writeSpecStats(int fds)
{
    if (!global::specificSearch)
        return;
    ecrire(fds, (char*)"\n\n<h2>Interesting pages (");
    ecrire(fds, global::contentTypes[0]);
    int i=1;
    while (global::contentTypes[i] != NULL)
    {
        ecrire(fds, (char*)", ");
        ecrire(fds, global::contentTypes[i]);
        i++;
    }
    ecrire(fds, (char*)") :</h2>\ntotal fetched (success)          : ");
    ecrireInti(fds, interestingPage, (char*)"%5d");
    ecrire(fds, (char*)"\ntotal fetched (error or success) : ");
    ecrireInti(fds, interestingSeen, (char*)"%5d");
    if (global::privilegedExts[0] != NULL)
    {
        ecrire(fds, (char*)"\nprivileged links seen (");
        ecrire(fds, global::privilegedExts[0]);
        uint i = 1;
        while (global::privilegedExts[i] != NULL)
        {
            ecrire(fds, (char*)", ");
            ecrire(fds, global::privilegedExts[i]);
            i++;
        }
        ecrire(fds, (char*)") :     ");
        ecrireInti(fds, interestingExtension, (char*)"%5d");
        ecrire(fds, (char*)"\nprivileged links fetched :         ");
        ecrireInti(fds, extensionTreated, (char*)"%5d");
    }
}

// main part
static void writeStats (int fds)
{
    HTTP("<div class=\"panel panel-primary\">\n");
    HTTP("<div class=\"panel-heading\">Stats</div>\n");
    HTTP("<div class=\"panel-body\">\n");
    HTTP("<table class=\"table table-condensed table-hover\">\n");
    HTTP("<thead>\n");
    HTTP("<tr>\n");
    HTTP("<th>Title</th>\n");
    HTTP("<th>Data</th>\n");
    HTTP("</tr>\n");
    HTTP("</thead>\n");
    HTTP("<tbody>\n");
    writeSpecStats(fds);
    HTTP("<tr>\n");
    HTTP("<td>urls treated</td>\n");
    HTTP("<td>");
    ecrireInti(fds, urls, (char*)"%1d");
    HTTP(" (current rate: ");
    ecrireInti(fds, urlsRate, (char*)"%d");
    HTTP(", overall rate: ");
    ecrireInti(fds, urls / totalduree, (char*)"%d");
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Pages</td>\n");
    HTTP("<td>");
    ecrireInti(fds, pages, (char*)"%d");
    ecrire(fds, (char*)" (current rate: ");
    ecrireInti(fds, pagesRate, (char*)"%d");
    ecrire(fds, (char*)", overall rate: ");
    ecrireInti(fds, pages / totalduree, (char*)"%d");
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Success</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[success], (char*)"%d");
    ecrire(fds, (char*)" (current rate: ");
    ecrireInti(fds, successRate, (char*)"%d");
    ecrire(fds, (char*)", overall rate: ");
    ecrireInti(fds, answers[success] / totalduree, (char*)"%d");
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("<div class=\"panel-group\" id=\"accordion\" role=\"tablist\" aria-multiselectable=\"true\">\n");
    HTTP("<div class=\"panel panel-info\">\n");
    HTTP("<div class=\"panel-heading\" role=\"tab\" id=\"headingOne\">\n");
    HTTP("<h4 class=\"panel-title\">\n");
    HTTP("<a data-toggle=\"collapse\" data-parent=\"#accordion\" href=\"#collapseOne\" aria-expanded=\"false\" aria-controls=\"collapseOne\">\n");
    HTTP("Error Statistics\n");
    HTTP("</a>\n");
    HTTP("</h4>\n");
    HTTP("</div>\n");
    HTTP("<div id=\"collapseOne\" class=\"panel-collapse collapse\" role=\"tabpanel\" aria-labelledby=\"headingOne\">\n");
    HTTP("<div class=\"panel-body\">\n");
    HTTP("<table class=\"table table-condensed table-hover\">\n");
    HTTP("<thead>\n");
    HTTP("<tr>\n");
    HTTP("<th>Title</th>\n");
    HTTP("<th>Data</th>\n");
    HTTP("</tr>\n");
    HTTP("</thead>\n");
    HTTP("<tbody>\n");
    HTTP("<tr>\n");
    HTTP("<td>forb robots</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[forbiddenRobots], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>no dns</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[noDNS], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>no connection</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[noConnection], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>early stop</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[earlyStop], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>timeout</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[timeout], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>bad type</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[badType], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>too big</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[tooBig], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>err 30X</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[err30X], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>err 40X</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[err40X], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    if (global::pageNoDuplicate)
    {
        HTTP("<tr>\n");
        HTTP("<td>duplicate</td>\n");
        HTTP("<td>");
        ecrireInti(fds, answers[duplicate], (char*)"%d");
        HTTP("</td>\n");
        HTTP("</tr>\n");
    }
    HTTP("<tr>\n");
    HTTP("<td>fast robots</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[fastRobots], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>fast no conn</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[fastNoConn], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>fast no dns</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[fastNoDns], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>too deep</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[tooDeep], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>dup url</td>\n");
    HTTP("<td>");
    ecrireInti(fds, answers[urlDup], (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("<div class=\"panel panel-info\">\n");
    HTTP("<div class=\"panel-heading\" role=\"tab\" id=\"headingTwo\">\n");
    HTTP("<h4 class=\"panel-title\">\n");
    HTTP("<a data-toggle=\"collapse\" data-parent=\"#accordion\" href=\"#collapseTwo\" aria-expanded=\"false\" aria-controls=\"collapseTwo\">\n");
    HTTP("DNS Statistics\n");
    HTTP("</a>\n");
    HTTP("</h4>\n");
    HTTP("</div>\n");
    HTTP("<div id=\"collapseTwo\" class=\"panel-collapse collapse\" role=\"tabpanel\" aria-labelledby=\"headingTwo\">\n");
    HTTP("<div class=\"panel-body\">\n");
    HTTP("<table class=\"table table-condensed table-hover\">\n");
    HTTP("<thead>\n");
    HTTP("<tr>\n");
    HTTP("<th>Title</th>\n");
    HTTP("<th>Data</th>\n");
    HTTP("</tr>\n");
    HTTP("</thead>\n");
    HTTP("<tbody>\n");
    HTTP("<tr>\n");
    HTTP("<td>Sites seen total number</td>\n");
    HTTP("<td>");
    ecrireInti(fds, siteSeen, (char*)"%d");
    HTTP(" + ");
    ecrireInti(fds, global::nbDnsCalls, (char*)"%d");
    HTTP(" (current rate: ");
    ecrireInti(fds, siteSeenRate, (char*)"%d");
    HTTP(", overall rate: ");
    ecrireInti(fds, siteSeen / totalduree, (char*)"%d");
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>with dns</td>\n");
    HTTP("<td>");
    ecrireInti(fds, siteDNS, (char*)"%d");
    HTTP(" (current rate: ");
    ecrireInti(fds, siteDNSRate, (char*)"%d");
    HTTP(", overall rate: ");
    ecrireInti(fds, siteDNS / totalduree, (char*)"%d");
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>with robots.txt</td>\n");
    HTTP("<td>");
    ecrireInti(fds, siteRobots, (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>with good robots.txt</td>\n");
    HTTP("<td>");
    ecrireInti(fds, robotsOK, (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>sites ready</td>\n");
    HTTP("<td>");
    ecrireInti(fds,
               global::okSites->getLength()
               + global::nb_conn
               - global::freeConns->getLength(), (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>sites without ip</td>\n");
    HTTP("<td>");
    ecrireInti(fds, global::dnsSites->getLength(), (char*)"%d");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("<p class=\"text-primary\"><strong>URLs accepted: ");
    ecrireInt(fds, hashUrls);
    HTTP(" / ");
    ecrireInt(fds, hashSize);
    HTTP("</strong></p>\n");
    HTTP("<div class=\"progress\">\n");
    HTTP("<div class=\"progress-bar progress-bar-success\" role=\"progressbar\" style=\"width: ");
    int rate = hashUrls / hashSize;
    ecrireInt(fds, rate);
    HTTP("%;\">");
    ecrireInt(fds, rate);
    HTTP("%</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
}

/** draw graphs
 */
static void writeGraph (int fds)
{
    if(global::histograms)
    {
        HTTP("<div class=\"panel panel-primary\">\n");
        HTTP("<div class=\"panel-heading\">Histograms</div>\n");
        HTTP("<div class=\"panel-body\">\n");
        HTTP("<div id=\"graphdiv\" style=\"width: 100%\"></div>\n");
        HTTP("</div>\n");
        HTTP("</div>\n");
        histoWrite(fds);
    }
}

/* write debug information (if any)
 */
static void writeDebug (int fds)
{
    if (!global::debug)
        return;
    ecrire(fds, (char*)"\n\n<h2>Ressources Sharing :</h2>\n\nused connexions : ");
    ecrireInti(fds, global::nb_conn - global::freeConns->getLength(), (char*)"%8d");
#ifdef THREAD_OUTPUT
    ecrire(fds, (char*)"\nuser connexions : ");
    ecrireInti(fds, global::userConns->getLength(), (char*)"%8d");
#endif
    ecrire(fds, (char*)"\nfree connexions : ");
    ecrireInti(fds, global::freeConns->getLength(), (char*)"%8d");
    ecrire(fds, (char*)"\nparsers         : ");
    ecrireInti(fds, debPars, (char*)"%8d");
    ecrire(fds, (char*)"\n\nsites in ram    : ");
    ecrireInti(fds, sites, (char*)"%8d");
    ecrire(fds, (char*)"\nipsites in ram  : ");
    ecrireInti(fds, ipsites, (char*)"%8d");
    ecrire(fds, (char*)"\nurls in ram     : ");
    ecrireInti(fds, debUrl, (char*)"%8d");
    ecrire(fds, (char*)"   (");
    ecrireInt(fds, global::inter->getPos() - space);
    ecrire(fds, (char*)" = ");
    ecrireInt(fds, namedUrl);
    ecrire(fds, (char*)" + ");
    ecrireInt(fds, global::IPUrl);
    ecrire(fds, (char*)")\nurls on disk    : ");
    int ui = global::URLsDisk->getLength();
    int uiw = global::URLsDiskWait->getLength();
    ecrireInti(fds, ui+uiw, (char*)"%8d");
    ecrire(fds, (char*)"   (");
    ecrireInt(fds, ui);
    ecrire(fds, (char*)" + ");
    ecrireInt(fds, uiw);
    ecrire(fds, (char*)")\npriority urls   : ");
    int up = global::URLsPriority->getLength();
    int upw = global::URLsPriorityWait->getLength();
    ecrireInti(fds, up+upw, (char*)"%8d");
    ecrire(fds, (char*)"   (");
    ecrireInt(fds, up);
    ecrire(fds, (char*)" + ");

    ecrireInt(fds, upw);
    ecrire(fds, (char*)")\nmiss urls       : ");
    ecrireInti(fds, missUrl, (char*)"%8d");

    ecrire(fds, (char*)"\n\nreceive : ");
    ecrireIntl(fds, byte_read, (char*)"%12lu");
    ecrire(fds, (char*)"   (current rate : ");
    ecrireIntl(fds, readRate, (char*)"%7lu");
    ecrire(fds, (char*)", overall rate : ");
    ecrireIntl(fds, byte_read / totalduree, (char*)"%7lu");
    ecrire(fds, (char*)")\nemit    : ");
    ecrireIntl(fds, byte_write, (char*)"%12lu");
    ecrire(fds, (char*)"   (current rate : ");
    ecrireIntl(fds, writeRate, (char*)"%7lu");
    ecrire(fds, (char*)", overall rate : ");
    ecrireIntl(fds, byte_write / totalduree, (char*)"%7lu");

    ecrire(fds, (char*)")\n\nstateMain : ");
    ecrireInt(fds, stateMain);
    ecrire(fds, (char*)"\ndebug     : ");
    ecrireInt(fds, debug);

#ifdef HAS_PROC_SELF_STATUS
    ecrire(fds, (char*)"\n\n<h2>/proc/self/status :</h2>\n");
    int status = open("/proc/self/status", O_RDONLY);
    char *file = readfile(status);
    ecrire(fds, file);
    delete [] file;
    close(status);
#endif // HAS_PROC_SELF_STATUS
}

/* write urls of the first dnsSites
 */
static void writeUrls (int fds)
{
    ecrire(fds, (char*)"<h2>Urls in next NamedSite</h2>\n");
    NamedSite *ds = global::dnsSites->tryRead();
    if (ds != NULL)
    {
        ecrireInt(fds, ds->fifoLength());
        ecrire(fds, (char*)" waiting url(s)\n\n");
        for (uint i=ds->outFifo; i!=ds->inFifo; i=(i+1)%maxUrlsBySite)
        {
            int n = ds->fifo[i]->writeUrl(buf);
            buf[n++] = '\n';
            ecrireBuff(fds, buf, n);
        }
    }
    else
    {
        ecrire(fds, (char*)"no site available\n");
    }
}

/* write urls of the first site in okSites
 */
static void writeIpUrls (int fds)
{
    ecrire(fds, (char*)"<h2>Urls in next IPSite</h2>\n");
    IPSite *is = global::okSites->tryRead();
    if (is != NULL)
    {
        Fifo<url> *f = &is->tab;
        ecrireInt(fds, f->getLength());
        ecrire(fds, (char*)" waiting url(s)\n\n");
        for (uint i=f->out; i!=f->in; i=(i+1)%f->size)
        {
            int n = f->tab[i]->writeUrl(buf);
            buf[n++] = '\n';
            ecrireBuff(fds, buf, n);
        }
    }
    else
    {
        ecrire(fds, (char*)"no site available\n");
    }
}

/* main function, manages the dispatch
 */
static void manageAns (int fds, char *req)
{
    if (req != NULL)
    {
        writeHeader(fds);
        if (!strncmp(req, "/output.html", 12))
        {
            outputStats(fds);
        }
        else if (!strcmp(req, "/dns.html"))
        {
            writeUrls(fds);
        }
        else if (!strcmp(req, "/urls.html"))
        {
            writeIpUrls(fds);
        }
        else if (!strcmp(req, "/all.html"))
        {
            writeStats(fds);
            writeGraph(fds);
            writeDebug(fds);
        }
        else if (!strcmp(req, "/system.html"))
        {
            writeDebug(fds);
        }
        else if (!strcmp(req, "/histograms.html"))
        {
            writeGraph(fds);
        }
        else if (!strcmp(req, "/stats.html"))
        {
            writeStats(fds);
        }
        else
        {
            writeStats(fds);
            writeGraph(fds);
        }
        writeFooter(fds);
    }
}

/******************************************************************/
// read request
static int pos;
static bool endFile;

/** parse and answer the current state
 * 1 : continue
 * 0 : end
 * -1 : error
 */
static int parseRequest (int size)
{
    while (size > 0)
    {
        if (pos < 4)
        {
            if (buf[pos] != "GET "[pos])
            {
                return -1;
            }
            else
            {
                pos++;
                size--;
            }
        }
        else
        {
            if (endFile)
            {
                // see if the request if finished
                pos += size;
                size = 0;
                if (buf[pos-1] == '\n' && buf[pos-3] == '\n')
                {
                    return 0;
                }
            }
            else
            {
                // find end of file
                char *tmp = strchr(buf+pos, ' ');
                if (tmp == NULL)
                {
                    pos += size;
                    return 1;
                }
                else
                {
                    *tmp = 0;
                    int i = pos;
                    pos = tmp + 1 - buf;
                    size = size + i - pos;
                    endFile = true;
                }
            }
        }
    }
    return 1;
}

static char *readRequest (int fds)
{
    pos = 0;
    endFile = false;
    while (true)
    {
        int size = read(fds, buf+pos, BUF_SIZE-pos);
        switch (size)
        {
        case -1:
        case 0:
            return NULL;
        default:
            buf[pos+size] = 0;
            int cont = parseRequest(size);
            if (cont == -1)
            {
                return NULL;
            }
            else if (cont == 0)
            {
                return buf+4;
            }
        }
    }
    // never reached, but avoid gcc warning
    return NULL;
}
