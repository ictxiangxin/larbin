Larbin网络爬虫
==================

[中文](/README-cn.md)    [English](/README.md)

版本：2.6.3

Larbin是一个网络爬虫，但是目前已经停止开发，最终的版本是2.6.3。
我将继续开发这一优秀的软件，并在Github上开放源代码。

开发概况：
----------

* 目前，我在Larbin v2.6.3上做了一些修改， 修复了许多编译时的警告，发现并修复了一个严重的bug。

* 最大的提升是我使用了最新版的adns v1.3替换了原始Larbin v2.6.3中的旧版adns v1.1。

内容提要：
----------

* [编译Larbin](#编译larbin)
* [配置Larbin](#配置larbin)
* [运行Larbin](#运行larbin)
* [支持的运行环境](#支持的运行环境)
* [联系我](#联系我)

###编译Larbin

查看options.h文件来进行功能选择。

```bash
./configure
make
```

###配置Larbin

配置larbin.conf. 将其修改为你的Email地址。

在doc文件夹下，有Larbin的html格式的详细文档。

###运行Larbin

确保你已经完成配置，然后运行：

```bash
./larbin
```

查看它的工作情况，访问http://localhost:8081/

###支持的运行环境

Larbin主要在Linux下进行开发。

我已经在Linux和FreeBSD下测试成功。

它可能在其他的平台下无法正确编译，但是我将在后续的版本中使其支持更多的平台。
请向我汇报Larbin在任何平台下的工作情况。

###联系我

URL: https://github.com/ictxiangxin/larbin

Email: ictxiangxin@gmail.com

QQ: 405340537
