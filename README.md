Larbin Web Crawler
==================

[中文](/README-cn.md) [English](/README.md)

Version: 2.6.3

Larbin is a web crawler and stop develop, the least version is 2.6.3.
We will continue develop this nice software and open the source on Github.

Develop:
--------

Currently, I did some modify on Larbin v2.6.3, fix many compiling warning and fix a serious bug.
The most improve is that I use the newest adns v1.3 to replace the old adns v1.1 which used in origin Larbin v2.6.3.

Table of content :
------------------

* [Compiling](#compiling)
* [Configuring](#configuring)
* [Running](#Running)
* [Supported platforms](#supported platforms)
* [Contact](#contact)

###Compiling

Have a look at options.h to choose options you need

```bash
./configure
make
```

###Configuring

See larbin.conf. Please be sure to specify your mail.

There is also some documentation in the doc directory in html format.

###Running

Be sure you did the configuration

```bash
./larbin
```

To see how it works, visit http://localhost:8081/

###Supported platforms

Larbin has mainly been developped under Linux.

I've tested larbin with success on Linux and freeBSD.

It probably won't compile right out of the box on any other platform,
but i'll work on it for future versions. Please report success or failure on any platform.

###Contact

URL: https://github.com/ictxiangxin/larbin

Email: ictxiangxin@gmail.com

QQ: 405340537
