Larbin Web Crawler
==================

[简体中文版](/README.md)

Version: 2.6.4

Larbin is a web crawler and stop develop, the least version is 2.6.3.
We will continue develop this nice software and open the source on Github.

Develop:
--------

* Currently, I did some modify on Larbin v2.6.3, fix many compiling warning and fix a serious bug.

* I use the newest adns v1.4 to replace the old adns v1.1 which used in origin Larbin v2.6.3.

* Add timer, you can set the uptime of Larbin.

* Add HighLevelWebServer mode, when larbin is end, webserver still running, return the state information.

* Add a new conf option: ignoreRobots, it can make Larbin ignore robots.txt, as you know.

* new version larbin will move all functions to larbin.conf file, instead old options.h file, you needn't recompile larbin to change it's functions.

Table of content :
------------------

* [Compiling](#compiling)
* [Configuring](#configuring)
* [Running](#running)
* [Platforms](#platforms)
* [TODO](#TODO)
* [Contact](#contact)

###Compiling

Have a look at options.h to choose options you need, these function directly affect the ablilities of your Larbin, please configure this file seriously. After you change this file each time, you should rebuild Larbin so that these options can go into effect.
(New version Larbin will give up use options.h file, all options will configured by larbin.conf)

Excute the next commands to compile:

```bash
> ./configure
> make
```
If you get error when doing configure, make sure that you have already installed makedepend, on Debian/Ubuntu class OS, you can use next commands to install it:
```bash
> sudo apt-get install xutils-dev
```
If you use SUSE:
```bash
> sudo zypper in makedepend
```
You also need to check that you have installed some basic tools, such as g++, m4, if not, on Debian/Ubuntu class OS, you can use next commands to install them:
```bash
> sudo apt-get install g++ m4
``` 
If you use SUSE:
```bash
> sudo zypper in gcc-g++ m4
```
Adns 1.4 need lynx when compiling, if missing error occur, please install it in the same way.

###Configuring

See larbin.conf. Please be sure to specify your mail.

Some websites resist crawler, you need modify the UserAgent entry to pose as browser.

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

###TODO

* First, I need to reconstruct the source code.
* A new webserver, I want to use GNU libmicrohttdp to implement it.
* Support javascript, this is a very big project.

###Contact

URL: https://github.com/ictxiangxin/larbin

Email: ictxiangxin@gmail.com

QQ: 405340537
