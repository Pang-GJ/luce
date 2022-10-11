#include "Socket.h"
#include "InetAddress.h"
#include "Logger.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>  // for TCP_NODELAY
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace luce::net {

Socket::~Socket() { close(sockfd_); }

void Socket::bindAddress(const InetAddress &localaddr) {
  int ret = bind(sockfd_, localaddr.getSockAddr(), sizeof(struct sockaddr_in));
  if (ret != 0) {
    LOG_FATAL("bind sockfd:%d failed\n", sockfd_);
  }
}

void Socket::listen() {
  int ret = ::listen(sockfd_, SOMAXCONN);
  if (ret != 0) {
    LOG_FATAL("listen sockfd:%d failed\n", sockfd_);
  }
}

int Socket::accept(InetAddress *peeraddr) {
  struct sockaddr_in addr;
  socklen_t len = sizeof addr;
  bzero(&addr, sizeof addr);

  // 这里需要设置非阻塞
  int connfd = ::accept4(sockfd_, (struct sockaddr *)(&addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd >= 0) {
    peeraddr->setSockAddr(addr);
  }

  return connfd;
}

void Socket::shutdownWrite() {
  if (::shutdown(sockfd_, SHUT_WR) < 0) {
    LOG_ERROR("shutdown write sockfd:%d error\n", sockfd_);
  }
}

void Socket::setTcpNoDelay(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof optval);
}

void Socket::setReuseAddr(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setReusePort(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
}

void Socket::setKeepAlive(bool on) {
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval);
}

}  // namespace luce::net