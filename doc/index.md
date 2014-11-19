#Larbin文档

##目录

* [larbin.conf简介](#larbin.conf简介)
* [larbin.conf详细配置](#larbin.conf详细配置)

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

###larbin.conf详细配置

一下配置说明中的名词对应与larbin-cn.conf中的注释。

####身份和基本配置

From 字段，用于配置使用者的电子邮件地址，通常不需要设置这个字段。

UserAgent 字段，该字段用于设置http头的Agent信息，如果需要伪装成主流浏览器的话，就需要修改该字段。

####输入和输入设置
