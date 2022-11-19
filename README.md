# luce
## 简介
基于`C++20 coroutine`的“高性能”epoll网络库，使用协程可以方便地用同步的代码写出异步的效果。

ps: “高性能”是指单看echo_server，性能跟`muduo`接近，没有进行更多的性能测试。  
同时`muduo`有很多内部的优化技巧，本项目暂未优化一些内部实现，所以有不少的提升空间。

### 期望
C++20带来了无栈协程，但是不方便普通用户使用，目前的标准库适合库作者使用。  
我想慢慢地封装一层协程异步框架，计划参考`rust`中的实现。

## 目前已实现的部分

- IO多路复用（epoll）+ MultiReactor
- C++20协程支持 co_await/co_return
    - 简单使用coro::Task<>即可令函数成为协程
- 动态线程池
- EventManager/Reactor
- TcpServer/TcpAcceptor/TcpConnection/TcpApplication
- [高性能异步日志](https://github.com/Pang-GJ/plog)
  - 没有合并异步日志，仍在开发中，计划封装一下`fmt`或者`std::format`TODO
- [json解析](https://github.com/Pang-GJ/tinyjson)
  - 为什么需要json解析：后期考虑基于本仓库开发类似`gin`的简易版HTTP框架

## 用法

参考`example/echo_server.cpp`  
用户需要继承`TcpApplication`(位于`include/net/tcp_application.hpp`)
实现自己的Tcp应用程序，  
具体来说要实现三个函数：

```cpp
 virtual coro::Task<> OnRequest(TcpConnectionPtr conn,
                                     TcpServer &server) = 0;
 virtual coro::Task<> OnOpen(TcpConnectionPtr conn) = 0;
 virtual coro::Task<> OnClose(TcpConnectionPtr conn) = 0;
```

`OnRequest`表示请求到来时的回调（已经使用协程尽量减少回调的使用，但总有一些不好去除的回调，如果你有更好的想法，欢迎跟我讨论）。  
`OnOpen`表示连接刚开启的回调  
`OnClose`表示连接关闭时的回调

## 网络模型

Reactor

用户所在的线程运行main_reactor，负责监听到来的请求连接，连接后交由sub_reactor进行读写

## RoadMap

- 协程
    - 目前的协程使用感觉还不是很容易
    - 缺少了不少组件，例如调度器、同步原语
- io_uring
- HTTP封装
  - 实现类似`gin`的简易HTTP框架
- 简单protobuf RPC实现
    - 更进一步可以使用自己实现的序列化
    - TODO: 不用写`IDL`的RPC
- Zero Copy
