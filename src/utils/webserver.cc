// Larbin
// Sebastien Ailleret
// 07-03-00 -> 17-03-02

/* This is the code for the webserver which is used 
 * for giving stats about the program while it is running
 *
 * It is very UNefficient, but i don't care since it should
 * not be used very often. Optimize here is a waste of time */

#ifndef NOWEBSERVER

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
#include "utils/connexion.h"
#include "utils/debug.h"
#include "utils/histogram.h"
#include "interf/useroutput.h"

static char *readRequest (int fds);
static void manageAns (int fds, char *req);

static time_t startTime;
static char *startDate;

// a buffer used for various things (read request, write urls...)
static char buf[BUF_SIZE];

void *startWebserver (void *none) {
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
  if ((fds = socket(AF_INET, SOCK_STREAM, 0)) == -1
	  || setsockopt(fds, SOL_SOCKET, SO_REUSEADDR, (char*)&nAllowReuse, sizeof(nAllowReuse))
	  || bind(fds, (struct sockaddr *) &addr, sizeof(addr)) != 0
	  || listen(fds, 4) != 0) {
	std::cerr << "Unable to get the socket for the webserver (" << global::httpPort
         << ") : " << strerror(errno) << std::endl;
	exit(1);
  }
  // answer requests
  for (;;) {
	struct sockaddr_in addrc;
	int fdc;
	uint len = sizeof(addr);
	fdc = accept(fds, (struct sockaddr *) &addrc, &len);
	if (fdc == -1) {
	  std::cerr << "Trouble with web server...\n";
	} else {
      manageAns(fdc, readRequest(fdc));
      close(fdc);
	}
  }
  return NULL;
}


////////////////////////////////////////////////////////////
// write answer

static void writeHeader(int fds) {
  ecrire(fds, (char*)"HTTP/1.0 200 OK\r\nServer: Larbin\r\nContent-type: text/html\r\n\r\n<html>\n<head>\n<title>");
  ecrire(fds, global::userAgent);
  ecrire(fds, (char*)" real time statistic</title>\n</head>\n<body bgcolor=\"#FFFFFF\">\n<center><h1>Larbin is up and running !</h1></center>\n<pre>\n");
}

static void writeFooter(int fds) {
  // end of page and kill the connexion
  ecrire(fds, (char*)"\n</pre>\n<a href=\"stats.html\">stats</a>\n(<a href=\"smallstats.html\">small stats</a>+<a href=\"graph.html\">graphics</a>)\n<a href=\"debug.html\">debug</a>\n<a href=\"all.html\">all</a>\n<a href=\"ip.html\">ip</a>\n<a href=\"dns.html\">dns</a>\n<a href=\"output.html\">output</a>\n<hr>\n<A HREF=\"mailto:ictxiangxin@gmail.com\">ictxiangxin@gmail.com</A>\n</body>\n</html>");
  shutdown(fds, 2);
}

static int totalduree;

/** write date and time informations */
static void writeTime (int fds) {
  ecrire(fds, (char*)"Start date :   ");
  ecrire(fds, startDate);
  ecrire(fds, (char*)"Current date : ");
  ecrire(fds, ctime(&global::now));
  ecrire(fds, (char*)"up: ");
  int duree = global::now - startTime;
  totalduree = duree;
  if (duree > 86400) {
    ecrireInt(fds, duree / 86400);
    duree %= 86400;
    ecrire(fds, (char*)" day(s) ");
  }
  ecrireInt(fds, duree/3600);
  duree %= 3600;
  ecrire(fds, (char*)":");
  ecrireInt2(fds, duree/60);
  duree %= 60;
  ecrire(fds, (char*)":");
  ecrireInt2(fds, duree);
}

/* write stats information (if any) */
#ifdef NOSTATS
#define writeStats(fds) ((void) 0)
#else

// specific part
#ifdef SPECIFICSEARCH
static void writeSpecStats(int fds) {
  ecrire(fds, (char*)"\n\n<h2>Interesting pages (");
  ecrire(fds, contentTypes[0]);
  int i=1;
  while (contentTypes[i] != NULL) {
    ecrire(fds, (char*)", ");
    ecrire(fds, contentTypes[i]);
    i++;
  }
  ecrire(fds, (char*)") :</h2>\ntotal fetched (success)          : ");
  ecrireInti(fds, interestingPage, (char*)"%5d");
  ecrire(fds, (char*)"\ntotal fetched (error or success) : ");
  ecrireInti(fds, interestingSeen, (char*)"%5d");
  if (privilegedExts[0] != NULL) {
    ecrire(fds, (char*)"\nprivileged links seen (");
    ecrire(fds, privilegedExts[0]);
    int i=1;
    while (privilegedExts[i] != NULL) {
      ecrire(fds, (char*)", ");
      ecrire(fds, privilegedExts[i]);
      i++;
    }
    ecrire(fds, (char*)") :     ");
    ecrireInti(fds, interestingExtension, (char*)"%5d");
    ecrire(fds, (char*)"\nprivileged links fetched :         ");
    ecrireInti(fds, extensionTreated, (char*)"%5d");
  }
}
#else
#define writeSpecStats(fds) ((void) 0)
#endif

// main part
static void writeStats (int fds) {
  writeSpecStats(fds);
  ecrire(fds, (char*)"\n\n<h2>Pages :</h2>\n\nurls treated : ");
  ecrireInti(fds, urls, (char*)"%13d");
  ecrire(fds, (char*)"   (current rate : ");
  ecrireInti(fds, urlsRate, (char*)"%3d");
  ecrire(fds, (char*)", overall rate : ");
  ecrireInti(fds, urls / totalduree, (char*)"%3d");
  ecrire(fds, (char*)")\nforb robots : ");
  ecrireInti(fds, answers[forbiddenRobots], (char*)"%14d");
  ecrire(fds, (char*)"\nno dns : ");
  ecrireInti(fds, answers[noDNS], (char*)"%19d");
  ecrire(fds, (char*)"\n\nPages : ");
  ecrireInti(fds, pages, (char*)"%20d");
  ecrire(fds, (char*)"   (current rate : ");
  ecrireInti(fds, pagesRate, (char*)"%3d");
  ecrire(fds, (char*)", overall rate : ");
  ecrireInti(fds, pages / totalduree, (char*)"%3d");
  ecrire(fds, (char*)")\nSuccess : ");
  ecrireInti(fds, answers[success], (char*)"%18d");
  ecrire(fds, (char*)"   (current rate : ");
  ecrireInti(fds, successRate, (char*)"%3d");
  ecrire(fds, (char*)", overall rate : ");
  ecrireInti(fds, answers[success] / totalduree, (char*)"%3d");
  ecrire(fds, (char*)")\nno connection : ");
  ecrireInti(fds, answers[noConnection], (char*)"%12d");
  ecrire(fds, (char*)"\nearly stop : ");
  ecrireInti(fds, answers[earlyStop], (char*)"%15d");
  ecrire(fds, (char*)"\ntimeout : ");
  ecrireInti(fds, answers[timeout], (char*)"%18d");
  ecrire(fds, (char*)"\nbad type : ");
  ecrireInti(fds, answers[badType], (char*)"%17d");
  ecrire(fds, (char*)"\ntoo big : ");
  ecrireInti(fds, answers[tooBig], (char*)"%18d");
  ecrire(fds, (char*)"\nerr 30X : ");
  ecrireInti(fds, answers[err30X], (char*)"%18d");
  ecrire(fds, (char*)"\nerr 40X : ");
  ecrireInti(fds, answers[err40X], (char*)"%18d");
#ifdef NO_DUP
  ecrire(fds, (char*)"\nduplicate : ");
  ecrireInti(fds, answers[duplicate], (char*)"%16d");
#endif // NO_DUP
  ecrire(fds, (char*)"\n\nurls accepted : ");
  ecrireInt(fds, hashUrls);
  ecrire(fds, (char*)" / ");
  ecrireInt(fds, hashSize);
  ecrire(fds, (char*)"\n\nfast robots : ");
  ecrireInti(fds, answers[fastRobots], (char*)"%14d");
  ecrire(fds, (char*)"\nfast no conn : ");
  ecrireInti(fds, answers[fastNoConn], (char*)"%13d");
  ecrire(fds, (char*)"\nfast no dns : ");
  ecrireInti(fds, answers[fastNoDns], (char*)"%14d");
  ecrire(fds, (char*)"\ntoo deep : ");
  ecrireInti(fds, answers[tooDeep], (char*)"%17d");
  ecrire(fds, (char*)"\ndup url : ");
  ecrireInti(fds, answers[urlDup], (char*)"%18d");

  ecrire(fds, (char*)"\n\n<h2>Sites seen (dns call done) :</h2>\n\ntotal number : ");
  ecrireInti(fds, siteSeen, (char*)"%18d");
  ecrire(fds, (char*)" +");
  ecrireInti(fds, global::nbDnsCalls, (char*)"%2d");
  ecrire(fds, (char*)"   (current rate : ");
  ecrireInti(fds, siteSeenRate, (char*)"%2d");
  ecrire(fds, (char*)", overall rate : ");
  ecrireInti(fds, siteSeen / totalduree, (char*)"%2d");
  ecrire(fds, (char*)")\nwith dns : ");
  ecrireInti(fds, siteDNS, (char*)"%22d");
  ecrire(fds, (char*)"       (current rate : ");
  ecrireInti(fds, siteDNSRate, (char*)"%2d");
  ecrire(fds, (char*)", overall rate : ");
  ecrireInti(fds, siteDNS / totalduree, (char*)"%2d");
  ecrire(fds, (char*)")\nwith robots.txt : ");
  ecrireInti(fds, siteRobots, (char*)"%15d");
  ecrire(fds, (char*)"\nwith good robots.txt : ");
  ecrireInti(fds, robotsOK, (char*)"%10d");

  ecrire(fds, (char*)"\n\nsites ready      : ");
  ecrireInti(fds,
             global::okSites->getLength()
             + global::nb_conn
             - global::freeConns->getLength(), (char*)"%5d");
  ecrire(fds, (char*)"\nsites without ip : ");
  ecrireInti(fds, global::dnsSites->getLength(), (char*)"%5d");
}
#endif // NOSTATS

/** draw graphs
 */
#if defined (NOSTATS) || !defined (GRAPH)
#define writeGraph(fds) ((void) 0)
#else
static void writeGraph (int fds) {
  ecrire(fds, (char*)"\n\n<h2>Histograms :</h2>");
  histoWrite(fds);
}
#endif

/* write debug information (if any)
 */
#ifdef NDEBUG
#define writeDebug(fds) ((void) 0)
#else
static void writeDebug (int fds) {
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
#endif // NDEBUG

/* write urls of the first dnsSites
 */
static void writeUrls (int fds) {
  ecrire(fds, (char*)"<h2>Urls in next NamedSite</h2>\n");
  NamedSite *ds = global::dnsSites->tryRead();
  if (ds != NULL) {
    ecrireInt(fds, ds->fifoLength());
    ecrire(fds, (char*)" waiting url(s)\n\n");
    for (uint i=ds->outFifo; i!=ds->inFifo; i=(i+1)%maxUrlsBySite) {
      int n = ds->fifo[i]->writeUrl(buf);
      buf[n++] = '\n';
      ecrireBuff(fds, buf, n);
    }
  } else {
    ecrire(fds, (char*)"no site available\n");
  }
}

/* write urls of the first site in okSites
 */
static void writeIpUrls (int fds) {
  ecrire(fds, (char*)"<h2>Urls in next IPSite</h2>\n");
  IPSite *is = global::okSites->tryRead();
  if (is != NULL) {
    Fifo<url> *f = &is->tab;
    ecrireInt(fds, f->getLength());
    ecrire(fds, (char*)" waiting url(s)\n\n");
    for (uint i=f->out; i!=f->in; i=(i+1)%f->size) {
      int n = f->tab[i]->writeUrl(buf);
      buf[n++] = '\n';
      ecrireBuff(fds, buf, n);
    }
  } else {
    ecrire(fds, (char*)"no site available\n");
  }
}

/* main function, manages the dispatch
 */
static void manageAns (int fds, char *req) {
  if (req != NULL) {
    writeHeader(fds);
    if (!strncmp(req, "/output.html", 12)) {
      outputStats(fds);
    } else if (!strcmp(req, "/dns.html")) {
      writeUrls(fds);
    } else if (!strcmp(req, "/ip.html")) {
      writeIpUrls(fds);
    } else if (!strcmp(req, "/all.html")) {
      writeTime(fds);
      writeStats(fds);
      writeGraph(fds);
      writeDebug(fds);
    } else if (!strcmp(req, "/debug.html")) {
      writeTime(fds);
      writeDebug(fds);
    } else if (!strcmp(req, "/graph.html")) {
      writeTime(fds);
      writeGraph(fds);
    } else if (!strcmp(req, "/smallstats.html")) {
      writeTime(fds);
      writeStats(fds);
    } else { // stat
      writeTime(fds);
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
static int parseRequest (int size) {
  while (size > 0) {
    if (pos < 4) {
      if (buf[pos] != "GET "[pos]) {
        return -1;
      } else {
        pos++; size--;
      }
    } else {
      if (endFile) {
        // see if the request if finished
        pos += size; size = 0;
        if (buf[pos-1] == '\n' && buf[pos-3] == '\n') {
          return 0;
        }
      } else {
        // find end of file
        char *tmp = strchr(buf+pos, ' ');
        if (tmp == NULL) {
          pos += size;
          return 1;
        } else {
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

static char *readRequest (int fds) {
  pos = 0;
  endFile = false;
  while (true) {
    int size = read(fds, buf+pos, BUF_SIZE-pos);
    switch (size) {
    case -1:
    case 0:
      return NULL;
    default:
      buf[pos+size] = 0;
      int cont = parseRequest(size);
      if (cont == -1) {
        return NULL;
      } else if (cont == 0) {
        return buf+4;
      }
    }
  }
  // never reached, but avoid gcc warning
  return NULL;
}

#endif // NOWEBSERVER
