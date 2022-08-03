#include "TcpServer.h"
#include <strings.h>
#include <sys/socket.h>
#include <cstdio>
#include <functional>
#include "Callbacks.h"
#include "Logger.h"
#include "TcpConnection.h"

EventLoop *CheckLoopNotNull(EventLoop *loop) {
  if (loop == nullptr) {
    LOG_FATAL("%s:%s:%d mainLoop is null \n", __FILE__, __FUNCTION__, __LINE__);
  }
  return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &name, Option option)
    : loop_(CheckLoopNotNull(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      started_(0), nextConnId_(1) {
  acceptor_->setNewConnectionCallback(
      std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
  for (auto &item : connections_) {
    // 利用智能指针，出了作用域将自动释放TcpConnection对象资源
    TcpConnectionPtr conn(item.second);
    item.second.reset();

    // 销毁连接
    conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int numThreads) { threadPool_->setThreadNum(numThreads); }

void TcpServer::start() {
  // 防止一个TcpServer对象被start()多次
  if (started_++ == 0) {
    threadPool_->start(threadInitCallback_);
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

// 每当有一个新的客户端连接，acceptor_会执行这个回调操作
void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
  // 轮询选择一个subLoop来管理channel
  EventLoop *ioLoop = threadPool_->getNextLoop();
  char buf[64] = {0};
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n", name_.c_str(), connName.c_str(),
           peerAddr.toIpPort().c_str());

  // 通过sockfd获取其绑定的本机ip地址和端口
  sockaddr_in local;
  ::bzero(&local, sizeof local);
  socklen_t addrlen = sizeof local;
  if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0) {
    LOG_ERROR("sockets::getLocalAddr \n");
  }
  InetAddress localAddr(local);

  // 根据连接成功的sockfd创建TcpConnection对象
  TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;
  // TcpServer=>TcpConnection=>Channel=>Poller=>notify
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);

  // 设置如何关闭连接的回调
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

  // 直接调用TcpConnection::connectEstablished
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n", name_.c_str(), conn->name().c_str());

  connections_.erase(conn->name());
  EventLoop *ioLoop = conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
