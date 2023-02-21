#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include "luce/common/noncopyable.h"

namespace net {

// 定义一个buffer存放数据的结构
class IOBuf {
 public:
  explicit IOBuf(size_t size);

  // 清空数据
  void Clear();

  // 将已经处理的数据清空
  // 未处理的数据移动至数据首地址
  void Adjust();

  // 将其它IOBuf的内容拷贝
  void Copy(const IOBuf *other);

  // 处理长度为len的数据，移动head和修正length
  void pop(size_t len);

  // 如果存在多个buffer，用链表连接起来
  IOBuf *next;
  // 当前buffer的容量大小
  size_t capacity;
  // 当前buffer的有效长度
  size_t length;
  // 未处理数据的头部位置索引
  size_t head;
  // 当前IOBuf保存的数据
  char *data;
};

using Pool = std::unordered_map<size_t, IOBuf *>;

// 总内存池最大限制 单位是kb 目前设为5GB
constexpr size_t MEM_LIMIT = static_cast<const size_t>(5U * 1024 * 1024);

// Buf内存池 采用单例模式
class BufPool : public noncopyable {
 public:
  enum MemCap {
    m4K = 4096,
    m16K = 16384,
    m64K = 65536,
    m256K = 262144,
    m1M = 1048576,
    m4M = 4194304,
    m8M = 8388608
  };

  static BufPool *GetInstance() {
    static BufPool buf_pool;
    return &buf_pool;
  }

  // 开辟一个IOBuf
  IOBuf *AllocBuf(int N = MemCap::m4K);

  // 重置一个IOBUf
  void Revert(IOBuf *buffer);

 private:
  BufPool();

  void AllocPoolMem(MemCap mem_cap, size_t num);

  Pool pool_;
  size_t total_mem_;
  // 单例对象
  // static BufPool *instance;
  std::mutex list_mtx_;
};

}  // namespace net
