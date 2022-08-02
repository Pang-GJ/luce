#pragma once

#include "Acceptor.h"
#include "Callbacks.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "noncopyable.h"

#include <atomic>
#include <functional>
#include <memory>
#include <unordered_map>

// 对外的服务器编程使用的类
class TcpServer : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop *)>;

  enum Option {
    kNoReusePort,
    kReusePort,
  };

  TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name, Option option = kNoReusePort);
  ~TcpServer();

  void setThreadInitCallback(const ThreadInitCallback &cb) { threadInitCallback_ = cb; }
  void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  // 设置底层subloop的个数
  void setThreadNum(int numThreads);

  // 开启服务器监听
  void start();

 private:
  using ConnctionMap = std::unordered_map<std::string, TcpConnectionPtr>;

  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnectionInLoop(const TcpConnectionPtr &conn);

  EventLoop *loop_;  // base loop  也就是 acceptor loop
  const std::string ipPort_;
  const std::string name_;

  std::unique_ptr<Acceptor> acceptor_;  // 运行在mainloop，监听新连接事件

  std::shared_ptr<EventLoopThreadPool> threadPool_;  // one loop per thread

  ConnectionCallback connectionCallback_;        // 有新连接时的回调
  MessageCallback messageCallback_;              // 有读写消息时的回调
  WriteCompleteCallback writeCompleteCallback_;  // 消息发送完成时的回调
  HighWaterMarkCallback highWaterMarkCallback_;
  size_t highWaterMark_;

  ThreadInitCallback threadInitCallback_;  // loop线程初始化的回调
  std::atomic_int started_;

  int nextConnId_;
  ConnctionMap connections_;  // 保存所有的连接
};
