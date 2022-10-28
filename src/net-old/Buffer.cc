#include "net/Buffer.h"

#include <sys/uio.h>
#include <unistd.h>
#include <cerrno>

namespace luce::net {

// 从fd上读取数据,Poller工作在LT模式
ssize_t Buffer::readFd(int fd, int *savedErrno) {
  char extrabuf[65536];  // 栈上的空间, 64k
  struct iovec vec[2];

  const size_t writable = writableBytes();
  vec[0].iov_base = begin() + writerIndex_;
  vec[0].iov_len = writable;

  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // 当buffer有足够的空间时，不会读到extrabuf中
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
  const ssize_t n = ::readv(fd, vec, iovcnt);

  if (n < 0) {
    *savedErrno = errno;

  } else if (n <= writable) {  // 此时extrabuf中不会有数据
    writerIndex_ += n;
  } else {
    // 此时extrabuf中有一部分数据
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);
  }

  return n;
}

ssize_t Buffer::writeFd(int fd, int *savedErrno) {
  ssize_t n = ::write(fd, peek(), readableBytes());
  if (n < 0) {
    *savedErrno = errno;
  }
  return n;
}

}  // namespace luce::net