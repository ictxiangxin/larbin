#Larbin Web Crawler

[![Build Status](https://travis-ci.org/ictxiangxin/larbin.svg?branch=master)](https://travis-ci.org/ictxiangxin/larbin)

[简体中文版](/README.md) |
[Documentation](/doc/index-en.md)

Version: 2.6.5

Larbin is a web crawler and stop develop, the least version is 2.6.3.
We will continue develop this nice software and open the source on Github.

##Develop:

* Currently, I did some modify on Larbin v2.6.3, fix many compiling warning and fix a serious bug.

* I use the newest adns v1.4 to replace the old adns v1.1 which used in origin Larbin v2.6.3.

* Add timer, you can set the uptime of Larbin.

* Add HighLevelWebServer mode, when larbin is end, webserver still running, return the state information.

* Add a new conf option: ignoreRobots, it can make Larbin ignore robots.txt, as you know.

* New version larbin will move all functions to larbin.conf file, instead old options.h file, you needn't recompile larbin to change it's functions.

* Supporting chinese, such as chinese words appear in URI and chinese domain name (e.g. [http://哈尔滨工业大学。中国](http://哈尔滨工业大学。中国)).

* Use cmake to build larbin

##Table of content :

* [Compiling](#compiling)
* [Configuring](#configuring)
* [Running](#running)
* [Platforms](#platforms)
* [TODO](#todo)
* [Contact](#contact)

###Compiling

New version larbin use cmake to build, and larbin wrote by C and C++, make sure that your system have been installed cmake and compiler.

If you have not cmake, on Debian/Ubuntu class OS, you can use next command to install it:
```bash
> sudo apt-get install cmake
```
If you use SUSE:
```bash
> sudo zypper in cmake
```
You also need to check that you have installed g++, if not, on Debian/Ubuntu class OS, you can use next command to install it:
```bash
> sudo apt-get install g++
``` 
If you use SUSE:
```bash
> sudo zypper in gcc-g++
```

I strongly recommend that use out-of-source build (depend on cmake), the particular build steps are:
```bash
> git clone https://github.com/ictxiangxin/larbin
> cd larbin
> mkdir build
> cmake ..
> make
```
If you want to make sure your larbin is OK, run:
```bash
> make test
```

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

By default, to see how it works, visit http://localhost:8081/

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
