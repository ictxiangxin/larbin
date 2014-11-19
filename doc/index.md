#Larbin文档

##目录

* [larbin.conf简介](#larbin.conf简介)
* [larbin.conf详细配置](/doc/configure.md)

###larbin.conf简介

larbin.conf是larbin的配置文件，由它控制larbin将如何工作以及做什么工作。

在新版本的larbin中，一共有3个版本的larbin.conf：

* larbin.conf
* larbin-cn.conf
* larbin-test.conf

其中larbin.conf和larbin-cn.conf默认的配置内容是完全一样的，唯一的区别是larbin-cn.conf使用中文注释。

larbin-test.conf是用于测试larbin正确构建而使用的，测试larbin是否正确构建，执行如下命令
```bash
> make test
```

larbin在运行时会默认选择larbin.conf为默认配置文件，如果要指定其他命名的配置文件（如：larbin-cn.conf），执行如下命令：
```bash
> ./larbin -c larbin-cn.conf
```
