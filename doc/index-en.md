#Larbin Documentation

##Contents

* [larbin.conf](#larbin.conf)

###larbin.conf

larbin.conf is the configure file of larbin, it will tell larbin how to work and working for what.

In new version larbin, there are 3 version larbin.conf:

* larbin.conf
* larbin-cn.conf
* larbin-test.conf

larbin.conf and larbin-cn.conf have the same configure contents, the only difference is that larbin-cn.conf use chinese as annotations.

larbin-test.conf is use for test whether larbin is been built correctly, to test whether larbin built correctly, execute following commands:
```bash
> make test
```

When larbin is running, it will use larbin.conf as the default configure file, If you want to appoint another configure file(such as larbin-cn.test), execute following commands:
```bash
> ./larbin -c larbin-cn.conf
```
