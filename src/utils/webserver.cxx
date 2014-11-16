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
#include "utils/limit_time.h"

#include "io/user_output.h"

#define HTTP(X) ecrire(fds, X)
#define HTTPint(X) ecrireInt(fds, X)


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

static void writeTime(int fds, time_t t)
{
    if (t > 86400)
    {
        ecrireInt(fds, t / 86400);
        t %= 86400;
        HTTP(" Day(s) ");
    }
    ecrireInt(fds, t / 3600);
    t %= 3600;
    HTTP(":");
    ecrireInt2(fds, t / 60);
    t %= 60;
    HTTP(":");
    ecrireInt2(fds, t);
}

static void writeHeader(int fds)
{
    HTTP("HTTP/1.0 200 OK\r\nServer: Larbin\r\nContent-type: text/html\r\n\r\n");
    HTTP("<html>\n");
    HTTP("<head>\n");
    HTTP("<link rel=\"stylesheet\" href=\"http://cdn.bootcss.com/bootstrap/3.3.0/css/bootstrap.min.css\">\n");
    HTTP("<script src=\"http://cdn.bootcss.com/jquery/1.11.1/jquery.min.js\"></script>\n");
    HTTP("<script src=\"http://cdn.bootcss.com/bootstrap/3.3.0/js/bootstrap.min.js\"></script>\n");
    HTTP("<script src=\"http://dygraphs.com/1.0.1/dygraph-combined.js\"></script>\n");
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
    HTTP("<a href=\"/\" class=\"navbar-brand\">Larbin</a>\n");
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
    writeTime(fds, duree);
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
    HTTP("<div class=\"panel panel-default\">\n");
    HTTP("<div class=\"panel-heading\" role=\"tab\" id=\"headingSpec\">\n");
    HTTP("<h4 class=\"panel-title\">\n");
    HTTP("<a data-toggle=\"collapse\" data-parent=\"#accordion\" href=\"#collapseSpec\" aria-expanded=\"false\" aria-controls=\"collapseSpec\">\n");
    HTTP("Specific Search\n");
    HTTP("</a>\n");
    HTTP("</h4>\n");
    HTTP("</div>\n");
    HTTP("<div id=\"collapseSpec\" class=\"panel-collapse collapse\" role=\"tabpanel\" aria-labelledby=\"headingSpec\">\n");
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
    HTTP("<td>Interesting Pages</td>\n");
    HTTP("<td>");
    HTTP(global::contentTypes[0]);
    uint i = 1;
    while (global::contentTypes[i] != NULL)
    {
        HTTP(", ");
        HTTP(global::contentTypes[i]);
        i++;
    }
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Total Fetched</td>\n");
    HTTP("<td>");
    HTTPint(interestingSeen);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Success</td>\n");
    HTTP("<td>");
    HTTPint(interestingPage);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    if (global::privilegedExts[0] != NULL)
    {
        HTTP("<tr>\n");
        HTTP("<td>Privileged Links Seen</td>\n");
        HTTP("<td>");
        HTTPint(interestingExtension);
        HTTP("</td>\n");
        HTTP("</tr>\n");
        HTTP("<tr>\n");
        HTTP("<td>Privileged Links Fetched</td>\n");
        HTTP("<td>");
        HTTPint(extensionTreated);
        HTTP("</td>\n");
        HTTP("</tr>\n");
        HTTP("<tr>\n");
        HTTP("<td>Extensions</td>\n");
        HTTP("<td>");
        ecrire(fds, global::privilegedExts[0]);
        uint i = 1;
        while (global::privilegedExts[i] != NULL)
        {
            HTTP(", ");
            HTTP(global::privilegedExts[i]);
            i++;
        }
        HTTP("</td>\n");
        HTTP("</tr>\n");
    }
    HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
}

// main part
static void writeStats (int fds)
{
    HTTP("<div class=\"panel panel-primary\">\n");
    HTTP("<div class=\"panel-heading\">Stats</div>\n");
    HTTP("<div class=\"panel-body\">\n");
    HTTP("<div class=\"row\">\n");
    HTTP("<div class=\"col-md-12\">\n");
    HTTP("<div class=\"btn-group\">\n");
    HTTP("<button href=\"#\" class=\"btn btn-primary active\">URLs Treaded</button>\n");
    HTTP("<button href=\"#\" class=\"btn btn-default active\">");
    HTTPint(urls);
    HTTP("</button>\n");
    HTTP("</div>\n");
    HTTP("<div class=\"btn-group\">\n");
    HTTP("<button href=\"#\" class=\"btn btn-info active\">Pages</button>\n");
    HTTP("<button href=\"#\" class=\"btn btn-default active\">");
    HTTPint(pages);
    HTTP("</button>\n");
    HTTP("</div>\n");
    HTTP("<div class=\"btn-group\">\n");
    HTTP("<button href=\"#\" class=\"btn btn-success active\">Success</button>\n");
    HTTP("<button href=\"#\" class=\"btn btn-default active\">");
    HTTPint(answers[success]);
    HTTP("</button>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("<br>\n");
    if (global::limitPage != 0)
    {
        HTTP("<p class=\"text-success\"><strong>Page Limit: ");
        ecrireInt(fds, answers[success]);
        HTTP(" / ");
        ecrireInt(fds, global::limitPage);
        HTTP("</strong></p>\n");
        int complete = (int)((float)answers[success] / (float)global::limitPage * 100);
        HTTP("<div class=\"progress\">\n");
        HTTP("<div class=\"progress-bar progress-bar-success\" role=\"progressbar\" style=\"width: ");
        ecrireInt(fds, complete);
        HTTP("%;\">");
        ecrireInt(fds, complete);
        HTTP("%</div>\n");
        HTTP("</div>\n");
    }
    if (global::limitTime != 0)
    {
        HTTP("<p class=\"text-info\"><strong>Time Limit: ");
        writeTime(fds, passTime());
        HTTP(" / ");
        writeTime(fds, global::limitTime);
        HTTP("</strong></p>\n");
        int complete = (int)((float)passTime() / (float)global::limitTime * 100);
        HTTP("<div class=\"progress\">\n");
        HTTP("<div class=\"progress-bar progress-bar-info\" role=\"progressbar\" style=\"width: ");
        ecrireInt(fds, complete);
        HTTP("%;\">");
        ecrireInt(fds, complete);
        HTTP("%</div>\n");
        HTTP("</div>\n");
    }
    HTTP("<div class=\"panel-group\" id=\"accordion\" role=\"tablist\" aria-multiselectable=\"true\">\n");
    writeSpecStats(fds);
    HTTP("<div class=\"panel panel-default\">\n");
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
    HTTP("<td>Forbidden Robots</td>\n");
    HTTP("<td>");
    HTTPint(answers[forbiddenRobots]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>No DNS</td>\n");
    HTTP("<td>");
    HTTPint(answers[noDNS]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>No Connection</td>\n");
    HTTP("<td>");
    HTTPint(answers[noConnection]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Early Stop</td>\n");
    HTTP("<td>");
    HTTPint(answers[earlyStop]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Timeout</td>\n");
    HTTP("<td>");
    HTTPint(answers[timeout]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Bad Type</td>\n");
    HTTP("<td>");
    HTTPint(answers[badType]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Too Big</td>\n");
    HTTP("<td>");
    HTTPint(answers[tooBig]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Error 30X</td>\n");
    HTTP("<td>");
    HTTPint(answers[err30X]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Error 40X</td>\n");
    HTTP("<td>");
    HTTPint(answers[err40X]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    if (global::pageNoDuplicate)
    {
        HTTP("<tr>\n");
        HTTP("<td>Duplicate</td>\n");
        HTTP("<td>");
        HTTPint(answers[duplicate]);
        HTTP("</td>\n");
        HTTP("</tr>\n");
    }
    HTTP("<tr>\n");
    HTTP("<td>Fast Robots</td>\n");
    HTTP("<td>");
    HTTPint(answers[fastRobots]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Fast No Connect</td>\n");
    HTTP("<td>");
    HTTPint(answers[fastNoConn]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Fast No DNS</td>\n");
    HTTP("<td>");
    HTTPint(answers[fastNoDns]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Too Deep</td>\n");
    HTTP("<td>");
    HTTPint(answers[tooDeep]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Duplicate URL</td>\n");
    HTTP("<td>");
    HTTPint(answers[urlDup]);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("<div class=\"panel panel-default\">\n");
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
    HTTP("<td>Sites Seen Total Number</td>\n");
    HTTP("<td>");
    HTTPint(siteSeen);
    HTTP(" + ");
    HTTPint(global::nbDnsCalls);
    HTTP(" (current rate: ");
    HTTPint(siteSeenRate);
    HTTP(", overall rate: ");
    HTTPint(siteSeen / totalduree);
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>With DNS</td>\n");
    HTTP("<td>");
    HTTPint(siteDNS);
    HTTP(" (current rate: ");
    HTTPint(siteDNSRate);
    HTTP(", overall rate: ");
    HTTPint(siteDNS / totalduree);
    HTTP(")");
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>With robots.txt</td>\n");
    HTTP("<td>");
    HTTPint(siteRobots);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>With Good robots.txt</td>\n");
    HTTP("<td>");
    HTTPint(robotsOK);
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Sites Ready</td>\n");
    HTTP("<td>");
    HTTPint(global::okSites->getLength()
          + global::nb_conn
          - global::freeConns->getLength());
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("<tr>\n");
    HTTP("<td>Sites Without IP</td>\n");
    HTTP("<td>");
    HTTPint(global::dnsSites->getLength());
    HTTP("</td>\n");
    HTTP("</tr>\n");
    HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");
    HTTP("<p class=\"text-primary\"><strong>URLs Accepted: ");
    ecrireInt(fds, hashUrls);
    HTTP(" / ");
    ecrireInt(fds, hashSize);
    HTTP("</strong></p>\n");
    int rate = (int)((float)hashUrls / (float)hashSize * 100);
    HTTP("<div class=\"progress\">\n");
    HTTP("<div class=\"progress-bar progress-bar-");
    if (rate <= 20)
        HTTP("success");
    else if (rate > 20 && rate <= 40)
        HTTP("info");
    else if (rate > 40 && rate <= 60)
        HTTP("primary");
    else if (rate > 60 && rate <= 80)
        HTTP("warning");
    else
        HTTP("danger");
    HTTP("\" role=\"progressbar\" style=\"width: ");
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
        HTTP("<div id=\"graphdiv3600\" style=\"width: 100%\"></div>\n");
        HTTP("<div id=\"graphdiv60\" style=\"width: 100%\"></div>\n");
        HTTP("<div id=\"graphdiv1\" style=\"width: 100%\"></div>\n");
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
	HTTP("<div class=\"panel panel-primary\">\n");
    HTTP("<div class=\"panel-heading\">Ressources Sharing</div>\n");
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
    HTTP("<td>Used Connections</td>\n");
	HTTP("<td>");
    HTTPint(global::nb_conn - global::freeConns->getLength());
	HTTP("</td>\n");
	HTTP("</tr>\n");
#ifdef THREAD_OUTPUT
    HTTP("<tr>\n");
    HTTP("<td>User Connections</td>\n");
	HTTP("<td>");
    HTTPint(global::userConns->getLength());
	HTTP("</td>\n");
	HTTP("</tr>\n");
#endif
    HTTP("<tr>\n");
    HTTP("<td>Free Connections</td>\n");
	HTTP("<td>");
    HTTPint(global::freeConns->getLength());
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
    HTTP("<td>Parsers</td>\n");
	HTTP("<td>");
    HTTPint(debPars);
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
    HTTP("<td>Sites in RAM</td>\n");
	HTTP("<td>");
    HTTPint(sites);
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
    HTTP("<td>IPsites in RAM</td>\n");
	HTTP("<td>");
    HTTPint(ipsites);
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
    HTTP("<td>URLs in RAM</td>\n");
	HTTP("<td>");
    HTTPint(debUrl);
    HTTP(" (");
    ecrireInt(fds, global::inter->getPos() - space);
    HTTP(" = ");
    ecrireInt(fds, namedUrl);
    HTTP(" + ");
    ecrireInt(fds, global::IPUrl);
    HTTP(")");
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
	HTTP("<td>URLs on Disk</td>\n");
    int ui = global::URLsDisk->getLength();
    int uiw = global::URLsDiskWait->getLength();
	HTTP("<td>");
    HTTPint(ui + uiw);
    HTTP(" (");
    ecrireInt(fds, ui);
    HTTP(" + ");
    ecrireInt(fds, uiw);
    HTTP(")");
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
	HTTP("<td>Priority URLs</td>\n");
    int up = global::URLsPriority->getLength();
    int upw = global::URLsPriorityWait->getLength();
	HTTP("<td>");
    HTTPint(up + upw);
    HTTP(" (");
    ecrireInt(fds, up);
    HTTP(" + ");
    ecrireInt(fds, upw);
    HTTP(")");
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
	HTTP("<td>Miss URLs</td>\n");
	HTTP("<td>");
    HTTPint(missUrl);
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
    HTTP("<td>Receive</td>\n");
	HTTP("<td>");
    ecrireIntl(fds, byte_read, (char*)"%lu");
    HTTP(" (current rate: ");
    ecrireIntl(fds, readRate, (char*)"%lu");
    HTTP(", overall rate: ");
    ecrireIntl(fds, byte_read / totalduree, (char*)"%lu");
    HTTP(")");
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
	HTTP("<td>Emit</td>\n");
	HTTP("<td>");
    ecrireIntl(fds, byte_write, (char*)"%lu");
    HTTP(" (current rate: ");
    ecrireIntl(fds, writeRate, (char*)"%lu");
    HTTP(", overall rate: ");
    ecrireIntl(fds, byte_write / totalduree, (char*)"%lu");
    HTTP(")");
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
	HTTP("<td>stateMain</td>\n");
	HTTP("<td>");
    ecrireInt(fds, stateMain);
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("<tr>\n");
    HTTP("<td>debug</td>\n");
	HTTP("<td>");
    ecrireInt(fds, debug);
	HTTP("</td>\n");
	HTTP("</tr>\n");
	HTTP("</tbody>\n");
    HTTP("</table>\n");
    HTTP("</div>\n");
    HTTP("</div>\n");

#ifdef HAS_PROC_SELF_STATUS
	HTTP("<div class=\"panel panel-primary\">\n");
    HTTP("<div class=\"panel-heading\">/proc/self/status</div>\n");
    HTTP("<div class=\"panel-body\">\n");
	HTTP("<table class=\"table table-condensed table-hover\">\n");
    HTTP("<thead>\n");
    HTTP("<tr>\n");
    HTTP("<th>Title</th>\n");
    HTTP("<th>Data</th>\n");
    HTTP("</tr>\n");
    HTTP("</thead>\n");
    HTTP("<tbody>\n");
    int status = open("/proc/self/status", O_RDONLY);
    char *file = readfile(status);
	int len = strlen(file);
	char tmp_entry[256] = {'\0'};
	int tmp_i = 0;
	int flag = 0;
	for (int i = 0; i < len; i++)
	{
	    if (file[i] == ':')
		{
		    tmp_entry[tmp_i] = '\0';
			tmp_i = 0;
			HTTP("<tr>\n");
			HTTP("<td>");
			HTTP(tmp_entry);
			HTTP("</td>\n");
			flag = 1;
			continue;
		}
		if (file[i] == '\n')
		{
		    tmp_entry[tmp_i] = '\0';
			tmp_i = 0;
			HTTP("<td>");
			HTTP(tmp_entry);
			HTTP("</td>\n");
			HTTP("</tr>\n");
		}
		if (flag == 1 && file[i] <= ' ')
		    continue;
		flag = 0;
	    tmp_entry[tmp_i++] = file[i];
	}
    //ecrire(fds, file);
    delete [] file;
    close(status);
	HTTP("</tbody>\n");
    HTTP("</table>\n");
	HTTP("</div>\n");
    HTTP("</div>\n");
#endif // HAS_PROC_SELF_STATUS
}

/* write urls of the first dnsSites
 */
static void writeUrls (int fds)
{
	HTTP("<div class=\"panel panel-primary\">\n");
    HTTP("<div class=\"panel-heading\">URLs in next NamedSite</div>\n");
    HTTP("<div class=\"panel-body\">\n");
    NamedSite *ds = global::dnsSites->tryRead();
    if (ds != NULL)
    {
	    HTTP("<span class=\"label label-info\">");
        ecrireInt(fds, ds->fifoLength());
		HTTP(" Waiting URLs</span>\n");
		HTTP("<table class=\"table table-condensed table-hover\">\n");
        HTTP("<thead>\n");
        HTTP("<tr>\n");
        HTTP("<th></th>\n");
        HTTP("</tr>\n");
        HTTP("</thead>\n");
        HTTP("<tbody>\n");
        for (uint i = ds->outFifo; i != ds->inFifo; i= (i + 1) % maxUrlsBySite)
        {
            int n = ds->fifo[i]->writeUrl(buf);
			HTTP("<tr>\n");
			HTTP("<td><a target=\"_blank\" href=\"");
            ecrireBuff(fds, buf, n);
			HTTP("\">");
			ecrireBuff(fds, buf, n);
			HTTP("</a></td>\n");
			HTTP("</tr>\n");
        }
    }
    else
    {
        HTTP("<span class=\"label label-primary\">No site available</span>\n");
    }
}

/* write urls of the first site in okSites
 */
static void writeIpUrls (int fds)
{
    HTTP("<div class=\"panel panel-primary\">\n");
    HTTP("<div class=\"panel-heading\">URLs in next IPSite</div>\n");
    HTTP("<div class=\"panel-body\">\n");
    IPSite *is = global::okSites->tryRead();
    if (is != NULL)
    {
        Fifo<url> *f = &is->tab;
		HTTP("<span class=\"label label-info\">");
        ecrireInt(fds, f->getLength());
		HTTP(" URLs</span>\n");
		HTTP("<table class=\"table table-condensed table-hover\">\n");
        HTTP("<thead>\n");
        HTTP("<tr>\n");
        HTTP("<th></th>\n");
        HTTP("</tr>\n");
        HTTP("</thead>\n");
        HTTP("<tbody>\n");
        for (uint i = f->out; i != f->in; i = (i + 1) % f->size)
        {
            int n = f->tab[i]->writeUrl(buf);
			HTTP("<tr>\n");
			HTTP("<td><a target=\"_blank\" href=\"");
            ecrireBuff(fds, buf, n);
			HTTP("\">");
			ecrireBuff(fds, buf, n);
			HTTP("</a></td>\n");
			HTTP("</tr>\n");
        }
		HTTP("</tbody>\n");
		HTTP("</table>\n");
    }
    else
    {
        HTTP("<span class=\"label label-primary\">No site available</span>\n");
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
