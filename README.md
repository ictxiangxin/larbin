#Larbin网络爬虫

[![Build Status](https://travis-ci.org/ictxiangxin/larbin.svg?branch=master)](https://travis-ci.org/ictxiangxin/larbin)

[See the English edition](/README-en.md)

版本：2.6.5

Larbin是一个网络爬虫，但是目前已经停止开发，最终的版本是2.6.3。
我将继续开发这一优秀的软件，并在Github上开放源代码。

[使用文档](/doc/index.md)

##开发概况：

* 目前，我在Larbin v2.6.3上做了一些修改， 修复了许多编译时的警告，发现并修复了一个严重的bug。

* 使用了最新版的adns v1.4替换了原始Larbin v2.6.3中的旧版adns v1.1。

* 增加了定时器功能，能设置larbin的运行时间。

* 增加HighLevelWebServer模式，在larbin停止爬取后，webserver依然正常运行，反馈状态信息。

* 增加了一个配置选项ignoreRobots，它可以使Larbin在抽取网页时不受robots.txt的影响，你懂的。

* 新版本larbin正在将所有功能开关以及设置移到larbin.conf中，而不是原来的options.h中, 你无需重新编译larbin就可以配置它的功能。

* 增加了对中文(unicode)的支持，包括URI中出现的中文编码以及中文域名（如：[http://哈尔滨工业大学。中国](http://哈尔滨工业大学。中国)）。

* 使用cmake来构建larbin。

##内容提要：

* [编译Larbin](#编译larbin)
* [配置Larbin](#配置larbin)
* [运行Larbin](#运行larbin)
* [运行环境](#运行环境)
* [计划开发](#计划开发)
* [联系我](#联系我)

###编译Larbin

新版larbin将使用cmake来构建，使用c和c++两种语言，请确保系统中已经安装了cmake以及编译器。

如果cmake没有安装，在Debian/Ubuntu类操作系统下，执行下列命令进行安装：
```bash
> sudo apt-get install cmake
```
如果你使用的系统是SUSE:
```bash
> sudo zypper in cmake
```
你仍然需要注意g++是否安装，如果没有，在Debian/Ubuntu类操作系统下，执行下列命令进行安装：
```bash
> sudo apt-get install g++
```
如果你使用的系统是SUSE:
```bash
> sudo zypper in gcc-g++
```

强烈建议larbin进行外部构建（由于cmake）， 具体安装步骤如下：
```bash
> git clone https://github.com/ictxiangxin/larbin
> cd larbin
> mkdir build
> cmake ..
> make
```
如果想确定larbin是否构建正确，执行：
```bash
> make test
```
现在，larbin已经构建完成并可以使用了。

###配置Larbin

配置larbin.conf。确保正确修改为你的Email地址。

部分站点会阻止爬虫，需要修改UserAgent来伪装成浏览器。

仔细逐行查看larbin.conf中的每一段话，它们将详细引导你如何正确配置Larbin并让它成功跑起来。

如果你想使用中文提示的配置文件，请配置larbin-cn.conf。

在doc文件夹下，有Larbin的html格式的详细文档。

###运行Larbin

确保你已经完成配置，然后运行：

```bash
> ./larbin
```
如果你使用的是中文配置文件larbin-cn.conf，那么你需要在运行时设置一下配置文件的文件名：
```bash
> ./larbin -c larbin-cn.conf
```
如果你自己编写了配置文件，那么也需要在运行时设置配置文件的文件名：
```bash
> ./larbin -c your_conf_filename
```

默认情况下，查看它的工作情况，访问http://localhost:8081/

###运行环境

Larbin主要在Linux下进行开发。

我已经在Linux和FreeBSD下测试成功。

它可能在其他的平台下无法正确编译，但是我将在后续的版本中使其支持更多的平台。
请向我汇报Larbin在任何平台下的工作情况。

###计划开发

* 首先还是要将代码重构一遍。
* 一个全新的webserver，计划使用GNU libmicrohttpd实现。
* 支持javascript，这将是一个非常庞大的功能。

###联系我

URL: https://github.com/ictxiangxin/larbin

Email: ictxiangxin@gmail.com

QQ: 405340537
