#include <iostream>
#include "common/thread_pool.hpp"

int main(int argc, char *argv[]) {
  // create thread pool with 4 worker threads
  ThreadPool pool(4, true);

  // enqueue and store future
  for (int i = 0; i < 100; ++i) {
    auto result = pool.Commit(
        [](int answer) {
          // TODO(pgj): use log
          std::cerr << "this thread id: " << std::this_thread::get_id()
                    << ", answer: " << answer << std::endl;
        },
        i);
  }

  return 0;
}
