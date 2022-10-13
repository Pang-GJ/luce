# luce
luce在意大利语中意味着“光”，这也是我对本项目的期望。

## 目前已实现的部分
- IO多路复用（epoll）+　MultiReactor
- 线程池
- EventLoop
- Basic Log
- json解析（未合并）

## RoadMap

- 协程
  - 参考点：
    - brpc bthread
    - PhotonLibOS
    - C++20 corotinue （目前决定基于C++20, 未来的趋势）
    - flare Fiber
- io_uring
- Async Log
- HTTP封装
- 简单protobuf RPC实现
  - 更进一步可以使用自己实现的序列化
- Zero Copy??

## 网络模型
Reactor or Proactor？
如果采用io_uring，是该用Reactor还是Proactor??
io_uring是异步IO，理论上应该用Proactor
还是两者都实现??（代码一致性如何保证???）