#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <cerrno>

#include "luce/common/logger.h"
#include "luce/net/socket.h"

namespace net {

Socket::~Socket() {
  if (fd_ != -1 && !is_closed_) {
    LOG_DEBUG("close fd = {}", fd_);
    ::close(fd_);
  }
}
void Socket::BindAddress(const InetAddress& local_addr) {
  int ret = ::bind(fd_, local_addr.GetSockAddr(), sizeof(struct sockaddr));
  if (ret != 0) {
    LOG_FATAL("bind sockfd: {} failed, errno: {}", fd_, errno);
  }
}
void Socket::Listen() {
  int ret = ::listen(fd_, SOMAXCONN);
  if (ret != 0) {
    LOG_FATAL("listen sockfd: {} failed, errno: {}", fd_, errno);
  }
}

void Socket::ShutdownWrite() {
  if (::shutdown(fd_, SHUT_WR) < 0) {
    LOG_ERROR("shutdown write sockfd: {} error\n", fd_);
  }
}

void Socket::SetTcpNoDelay(bool on) {
  int option = on ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &option, sizeof option);
}

void Socket::SetReuseAddr(bool on) {
  int option = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option);
}

void Socket::SetReusePort(bool on) {
  int option = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &option, sizeof option);
}

void Socket::SetKeepAlive(bool on) {
  int option = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &option, sizeof option);
}
void Socket::SetNonblock() {
  auto flag = fcntl(fd_, F_GETFL);
  fcntl(fd_, F_SETFL, flag | O_NONBLOCK);
}

}  // namespace net
