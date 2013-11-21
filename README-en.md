Larbin Web Crawler
==================

[简体中文版](/README.md)

Version: 2.6.3

Larbin is a web crawler and stop develop, the least version is 2.6.3.
We will continue develop this nice software and open the source on Github.

Develop:
--------

* Currently, I did some modify on Larbin v2.6.3, fix many compiling warning and fix a serious bug.

* The most improve is that I use the newest adns v1.3 to replace the old adns v1.1 which used in origin Larbin v2.6.3.

* Add a new conf option: ignoreRobots, it can make Larbin ignore robots.txt, as you know.

Table of content :
------------------

* [Compiling](#compiling)
* [Configuring](#configuring)
* [Running](#running)
* [Platforms](#platforms)
* [Contact](#contact)

###Compiling

Have a look at options.h to choose options you need, these function directly affect the ablilities of your Larbin, please configure this file seriously. After you change this file each time, you should rebuild Larbin so that these options can go into effect.

Excute the next commands to compile:

```bash
> ./configure
> make
```
If you get error when doing configure, make sure that you have already installed makedepend, on Debian/Ubuntu class OS, you can use next commands to install it:
```bash
> sudo apt-get install xutils-dev
```
You also need to check that you have installed some basic tools, such as g++, m4, if not, on Debian/Ubuntu class OS, you can use next commands to install them:
```bash
> sudo apt-get install g++ m4
``` 

###Configuring

See larbin.conf. Please be sure to specify your mail.

Read the sentences of larbin.conf line by line, they will lead you configure your Larbin accurately and let your Larbin running successfully.

If you want to use chinese configure file, please see larbin-cn.conf.

There is also some documentation in the doc directory in html format.

###Running

Be sure you did the configuration

```bash
> ./larbin
```
If you use the chinese configure file larbin-cn.conf, then you should set the configure filename when you start Larbin:
```bash
> ./larbin -c larbin-cn.conf
```
If you write a new configure file, then you also need set the configure filename when you start Larbin:
```bash
> ./larbin -c your_conf_filename
```

To see how it works, visit http://localhost:8081/

###Platforms

Larbin has mainly been developped under Linux.

I've tested larbin with success on Linux and freeBSD.

It probably won't compile right out of the box on any other platform,
but i'll work on it for future versions. Please report success or failure on any platform.

###Contact

URL: https://github.com/ictxiangxin/larbin

Email: ictxiangxin@gmail.com

QQ: 405340537
