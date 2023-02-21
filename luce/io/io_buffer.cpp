#include "luce/common/logger.hpp"
#include "luce/io/io_buffer.hpp"
#include <cassert>
#include <cstring>

namespace net {

IOBuf::IOBuf(size_t size) : next(nullptr), capacity(size), length(0), head(0) {
  data = new char[size];
  assert(data);
}

// 清空数据
void IOBuf::Clear() { length = head = 0; }

void IOBuf::Adjust() {
  if (head != 0) {
    if (length != 0) {
      std::memmove(data, data + head, length);
    }
    head = 0;
  }
}

void IOBuf::Copy(const IOBuf *other) {
  std::memcpy(data, other->data + other->head, other->length);
  head = 0;
  length = other->length;
}

void IOBuf::pop(size_t len) {
  length -= len;
  head += len;
}

BufPool::BufPool() : total_mem_(0) {
  // 4K的IOBuf开辟5000个，约20MB
  AllocPoolMem(m4K, 5000);
  total_mem_ += 4L * 5000;

  // 16K的IOBuf开辟1000个，约16MB
  AllocPoolMem(m16K, 1000);
  total_mem_ += 16L * 1000;

  // 64K开辟500个，约32MB
  AllocPoolMem(m64K, 500);
  total_mem_ += 64L * 500;

  // 256K开辟200个, 约50MB
  AllocPoolMem(m256K, 200);
  total_mem_ += 256L * 200;

  // 1M开辟50个，50MB
  AllocPoolMem(m1M, 50);
  total_mem_ += 1024L * 50;

  // 4M开辟20个，80MB
  AllocPoolMem(m4M, 20);
  total_mem_ += 4L * 1024 * 20;

  // 8M开辟10个, 80MB
  AllocPoolMem(m8M, 10);
  total_mem_ += 8L * 1024 * 10;
}

void BufPool::AllocPoolMem(MemCap mem_cap, size_t num) {
  IOBuf *prev;
  // 开辟mem_cap大小的 buf 内存池
  pool_[mem_cap] = new IOBuf(mem_cap);
  if (pool_[mem_cap] == nullptr) {
    LOG_ERROR("new IOBuf error, target size: {}", mem_cap);
    exit(1);
  }

  prev = pool_[mem_cap];
  // 开辟num个
  for (size_t i = 1; i < num; ++i) {
    prev->next = new IOBuf(mem_cap);
    if (prev->next == nullptr) {
      LOG_ERROR("new IOBuf error, target size: {}", mem_cap);
      exit(1);
    }
    prev = prev->next;
  }
}

// 开辟一个IOBuf
// 1. 如果上层需要N个字节大小的空间，找到与N最近的buf hash组，取出
// 2. 如果该组已经没有节点可以使用，可以额外申请
// 3. 总申请长度不能超过最大的限制大小 MEM_LIMIT
// 4. 如果有该节点需要的内存块, 直接取出，并将该内存块从pool中摘除
IOBuf *BufPool::AllocBuf(int N) {
  // 找到N最接近哪个hash组
  int key;
  if (N <= m4K) {
    key = m4K;
  } else if (N <= m16K) {
    key = m16K;
  } else if (N <= m64K) {
    key = m64K;
  } else if (N <= m256K) {
    key = m256K;
  } else if (N <= m1M) {
    key = m1M;
  } else if (N <= m4M) {
    key = m4M;
  } else if (N <= m8M) {
    key = m8M;
  } else {
    return nullptr;
  }

  // 如果该组已经没有节点，需要额外申请
  std::lock_guard<std::mutex> lock(list_mtx_);
  if (pool_[key] == nullptr) {
    if (total_mem_ + key / 1024 >= MEM_LIMIT) {
      // 要开辟的空间超过最大限制
      LOG_ERROR("use too many memory!");
      exit(1);
    }

    IOBuf *new_buf = new IOBuf(key);
    if (new_buf == nullptr) {
      LOG_ERROR("new buf error");
      exit(1);
    }
    total_mem_ += key / 1024;
    return new_buf;
  }

  // 如果有，从pool_中摘除该内存块
  IOBuf *target = pool_[key];
  pool_[key] = target->next;
  target->next = nullptr;
  return target;
}

// 重置一个IOBuf, 上层不再使用或者使用完成之后
// 需要将该Buf放回pool中
void BufPool::Revert(IOBuf *buffer) {
  // 每个buf的容量是固定的，它们的大小就是pool中的key
  auto key = buffer->capacity;
  // 重置IOBuf内的位置指针
  buffer->head = 0;
  buffer->length = 0;

  std::lock_guard<std::mutex> lock(list_mtx_);
  assert(pool_.find(key) != pool_.end());
  // 将buffer插回链表头部
  buffer->next = pool_[key];
  pool_[key] = buffer;
}

}  // namespace net
