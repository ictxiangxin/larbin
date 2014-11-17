#Larbin网络爬虫

[![Build Status](https://travis-ci.org/ictxiangxin/larbin.svg?branch=master)](https://travis-ci.org/ictxiangxin/larbin)

[See the English edition](/README-en.md) |
[文档](/doc/index.md)

版本：2.6.5

Larbin是一个网络爬虫，但是目前已经停止开发，最终的版本是2.6.3。
我将继续开发这一优秀的软件，并在Github上开放源代码。

##截图展示：

###状态界面
![状态界面](http://raw.github.com/ictxiangxin/larbin/master/doc/image/readme/state.jpg)

###图表界面
![图表界面](http://raw.github.com/ictxiangxin/larbin/master/doc/image/readme/histogram.jpg)

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
> cd build
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
* 支持javascript，这将是一个非常庞大的功能。

###联系我

URL: https://github.com/ictxiangxin/larbin

Email: ictxiangxin@gmail.com

QQ: 405340537
