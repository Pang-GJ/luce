#include <fmt/chrono.h>
#include <fmt/core.h>
#include <iostream>
#include <thread>
#include "luce/coro/task.hpp"

coro::Task<int> simple_task2() {
  fmt::print("task 2 start...\n");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(1s);
  fmt::print("task 2 returns after 1s\n");
  co_return 2;
}

coro::Task<int> simple_task3() {
  fmt::print("task 3 start...\n");
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
  fmt::print("task 3 returns after 2s\n");
  co_return 3;
}

coro::Task<int> simple_task() {
  fmt::print("task start...\n");
  auto result2 = co_await simple_task2();
  fmt::print("waiting for result2...\n");
  fmt::print("result from task2: {}\n", result2);
  auto result3 = co_await simple_task3();
  fmt::print("result from task3: {}\n", result3);
  co_return 1 + result2 + result3;
}

coro::Task<int> ans() { co_return 42; }

int main(int argc, char *argv[]) {
  auto task = simple_task();
  fmt::print("the result of simple_task: {}\n", task.GetResult());
  return 0;
}
