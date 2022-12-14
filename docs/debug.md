# BUG解决记录

## Task异常

gdb调试bt查看堆栈后发现是`std::future`的exception_ptr问题，应该是异常问题，但我没咋用过异常（Google
style不允许使用异常，就没关注过）。
所以我解决办法是直接把`std::future`去掉了，换成朴素点的实现

## echo_server莫名挂掉

gdb启动调试，发现挂掉是因为`SITPIPE`信号。
具体原因参考资料：

1. https://www.cnblogs.com/lit10050528/p/5116566.html
2. http://senlinzhan.github.io/2017/03/02/sigpipe/
   解决办法是忽略`SIGPIPE`信号

## echo_server进行bench的时候，测试工具显示echo_server返回的数据长度不对

测试工具发送的是512bytes的数据，但接收到的部分数据大小有40-490的都有。
原因：TCP粘包问题
最开始echo_server的buffer大小设置为了1024字节

## 性能测试低

跟muduo一起对比了echo_server的性能，发现我这个demo性能跟muduo单线程的性能差不多(
14w/s)，但muduo开10个线程时性能是单线程的接近5倍。
分析了一下目前我这个IO库，是单线程的，CPU利用率上不去，所以性能低
可以考虑增加一个“协程池”？就是各个协程放在不同的线程去跑

## Arch Linux core dump文件在哪？

详细教程：https://www.christian-schneider.it/blog/analyzing-linux-coredump/

## Interrupted system call

epoll_await返回-1时出现了EINTR。  
我们在利用 gdb 调试带有 epoll_wait select sem_wat 的多线程代码的时候可能会出现非正常返回
-1 的情况，错误原因是：Interrupted system call。  
这是由于 gdb调试的时候会在断点处插入一条中断指令,当程序执行到该断点处的时候会发送一个SIGTRAP信号,程序转去执行中断相应,进而gdb让程序停下来进行调试.
对于sem_wait\wait\read等会阻塞的函数在调试时,如果阻塞,都可能会收到调试器发送的信号,而返回非0值.