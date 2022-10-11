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
    - C++20 corotinue
    - flare Fiber
- io_uring
- HTTP封装
- 简单protobuf RPC实现
  - 更进一步可以使用自己实现的序列化