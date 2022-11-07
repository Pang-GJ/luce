# BUG解决记录

## Task异常
gdb调试bt查看堆栈后发现是`std::future`的exception_ptr问题，应该是异常问题，但我没咋用过异常（Google style不允许使用异常，就没关注过）。
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
跟muduo一起对比了echo_server的性能，发现我这个demo性能跟muduo单线程的性能差不多(14w/s)，但muduo开10个线程时性能是单线程的接近5倍。
分析了一下目前我这个IO库，是单线程的，CPU利用率上不去，所以性能低
可以考虑增加一个“协程池”？就是各个协程放在不同的线程去跑
