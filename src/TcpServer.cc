#include "TcpServer.h"
#include "Logger.h"

EventLoop *CheckLoopNotNull(EventLoop *loop) {
  if (loop == nullptr) {
    LOG_FATAL("%s:%s:%d mainLoop is null \n", __FILE__, __FUNCTION__, __LINE__);
  }
  return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, 
                     const std::string &name, Option option) 
  : loop_(CheckLoopNotNull(loop)), ipPort_(listenAddr.toIpPort()),
     name_(name), acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)), 
    threadPool_(new EventLoopThreadPool(loop, name_)), 
    connectionCallback_(),
    messageCallback_(),
    nextConnId_(1)
{
  acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, 
                                  this, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {

}

void TcpServer::setThreadNum(int numThreads) {
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
  // 防止一个TcpServer对象被start()多次
  if (started_++ == 0) {
    threadPool_->start(threadInitCallback_);
    loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}
