#pragma once

#include <unistd.h>
#include <sys/syscall.h>
#include <cstdint>

namespace CurrentThread {
extern __thread int t_cachedTid;

void cacheTid();

// 获取当前线程的tid
inline int tid() {
  if (__builtin_expect( static_cast<int64_t>(t_cachedTid == 0), 0) != 0) {
    cacheTid();
  }

  return t_cachedTid;
}

}  // namespace CurrentThread
