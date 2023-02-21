#include <iostream>
#include <thread>
#include "luce/common/logger.hpp"
#include "luce/common/thread_pool.hpp"

int main(int argc, char *argv[]) {
  // create thread pool with 4 worker threads
  ThreadPool pool(4);

  // enqueue and store future
  for (int i = 0; i < 100; ++i) {
    auto result = pool.Commit(
        [](int answer) {
          // TODO(pgj): use log
          std::stringstream ss;
          ss << std::this_thread::get_id();
          LOG_INFO("this_thread: {}, answer: {}", ss.str(), answer);
        },
        i);
  }

  return 0;
}
