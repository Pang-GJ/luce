#pragma once

// 网络库底层的缓冲区
#include <unistd.h>
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

class Buffer {
 public:
  static const size_t kCheapPrepend = 8;
  static const size_t kInitialSize = 1024;

  explicit Buffer(size_t initialSize = kInitialSize)
      : buffer_(initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend) {}

  size_t readableBytes() const { return writerIndex_ - readerIndex_; }
  size_t writableBytes() const { return buffer_.size() - writerIndex_; }
  size_t prependableBytes() const { return readerIndex_; }

  // 返回缓冲区中可读数据的起始地址
  const char *peek() const { return begin() + readerIndex_; }

  void retrieve(size_t len) {
    if (len < readableBytes()) {
      readerIndex_ += len;

    } else {  // len == readableBytes()
      retrieveAll();
    }
  }

  void retrieveAll() { readerIndex_ = writerIndex_ = kCheapPrepend; }

  // 将buffer的数据转换为string类型的数据返回
  std::string retrieveAllAsString() { return retrieveAsString(readableBytes()); }

  std::string retrieveAsString(size_t len) {
    // FIXME(pgj): assert len <= readableBytes
    std::string result(peek(), len);
    retrieve(len);  // 上面代码读出了缓冲区的数据，此时读指针要跳过读了的部分
    return result;
  }

  void ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
      makeSpace(len);  // 扩容函数
    }
  }

  // 把[data, data+len]内存上的数据，添加到writable缓冲区中
  void append(const char *data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    writerIndex_ += len;
  }

  // Read data directly into buffer
  ssize_t readFd(int fd, int *savedErrno);
  ssize_t writeFd(int fd, int *savedErrno);

  char *beginWrite() { return begin() + writerIndex_; }

  const char *beginWrite() const { return begin() + writerIndex_; }

 private:
  char *begin() {
    return &(*buffer_.begin());  // vector底层数组首元素的地址，也就是vector的首地址
  }

  const char *begin() const { return &(*buffer_.begin()); }

  void makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
      // FIXME(pgj): move readable data
      buffer_.resize(writableBytes() + len);
    } else {
      // move readable data to front, make space inside buffer
      size_t readable = readableBytes();
      std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
      readerIndex_ = kCheapPrepend;
      writerIndex_ = readerIndex_ + readable;
    }
  }

  std::vector<char> buffer_;
  size_t readerIndex_;  // 读指针
  size_t writerIndex_;  // 写指针
};
