#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include "luce/common/logger.hpp"

// 线程池
class ThreadPool {
 public:
  explicit ThreadPool(size_t thread_num);
  ~ThreadPool();

  template <class F, class... Args>
  auto Commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;

  void Shutdown();

  size_t Size() const { return workers_.size(); }

 private:
  std::vector<std::thread> workers_;
  std::queue<std::packaged_task<void()>> tasks_;

  // sync
  std::mutex queue_mtx_;
  std::condition_variable condition;
  std::condition_variable condition_producers_;
  std::atomic<bool> stop_;
};

inline ThreadPool::ThreadPool(size_t thread_num) : stop_(false) {
  for (size_t i = 0; i < thread_num; ++i) {
    workers_.emplace_back([this] {
      for (;;) {
        std::packaged_task<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mtx_);
          this->condition.wait(
              lock, [this] { return this->stop_ || !this->tasks_.empty(); });
          if (this->stop_) {
            return;
          }
          if (this->tasks_.empty()) {
            continue;
          }
          task = std::move(this->tasks_.front());
          this->tasks_.pop();
          if (this->tasks_.empty()) {
            // notify that the queue is empty
            condition_producers_.notify_one();
          }
        }
        task();
      }
    });
  }
}

inline ThreadPool::~ThreadPool() {
  if (!stop_) {
    Shutdown();
  }
}

// add new work item to thead pool
template <class F, class... Args>
inline auto ThreadPool::Commit(F &&f, Args &&...args)
    -> std::future<decltype(f(args...))> {
  using return_type = decltype(f(args...));

  std::packaged_task<return_type()> task(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));
  std::future<return_type> future = task.get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mtx_);
    if (!stop_) {
      tasks_.emplace(std::move(task));
    }
  }
  condition.notify_one();
  return future;
}

inline void ThreadPool::Shutdown() {
  LOG_INFO("thread pool shutdown");
  {
    std::unique_lock<std::mutex> lock(queue_mtx_);
    condition_producers_.wait(lock, [this] { return this->tasks_.empty(); });
    stop_ = true;
  }
  condition.notify_all();
  for (std::thread &worker : workers_) {
    worker.join();
  }
}

// 可以增长 method: AddThread
// 可选支持自动扩缩容
// 支持多参数
/**
class ThreadPool {
  using TaskType = std::function<void()>;

 public:
  explicit ThreadPool(size_t init_size = 1, bool auto_grow = false,
                      size_t max_threads =
std::thread::hardware_concurrency()) : max_threads_num_(max_threads),
        init_threads_num_(init_size),
        auto_grow_(auto_grow) {
    AddThread(init_threads_num_);
  }

  ~ThreadPool() {
    if (!is_shutdown_) {
      Shutdown();
    }
  }

  void Shutdown() {
    is_shutdown_ = true;
    // 唤醒所有线程
    tasks_cv_.notify_all();
    for (auto &worker : workers_) {
      if (worker.joinable()) {
        worker.join();
      }
    }
  }

  void AddThread(size_t num) {
    if (is_shutdown_) {
      return;
    }

    std::unique_lock<std::mutex> grow_lock(grow_mtx_);
    if (!auto_grow_) {
      grow_lock.unlock();
    }

    for (size_t i = 0; i < num && workers_.size() < max_threads_num_; ++i) {
      workers_.emplace_back([this] { this->work(); });

      std::lock_guard<std::mutex> lock(mtx_);
      free_thread_num_++;
    }
  }

  // 提交一个任务
  // 调用.get()获取返回值会等待任务执行完,获取返回值
  template <typename F, typename... Args>
  auto Commit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
    if (is_shutdown_) {
      LOG_FATAL("ThreadPool is Shutdown");
    }

    using ReturnType = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto future = task->get_future();
    {
      // 将任务添加到任务队列
      std::lock_guard<std::mutex> lock(mtx_);
      tasks_.emplace([task]() { (*task)(); });
    }

    if (auto_grow_) {
      // 判断是否需要自动扩容
      if (free_thread_num_ < 1 && workers_.size() < max_threads_num_) {
        AddThread(1);
      }
    }

    // 唤醒一个线程执行
    tasks_cv_.notify_one();
    return future;
  }

  // 空闲线程数量
  size_t FreeSize() { return free_thread_num_; }

  // 线程数量
  size_t Size() {
    std::lock_guard<std::mutex> lock(grow_mtx_);
    auto sz = workers_.size();
    return sz;
  }

 private:
  void work() {
    // 防止 is_shutdown_==true 时立即结束,此时任务队列可能不为空
    while (true) {
      TaskType task;
      {
        std::unique_lock<std::mutex> lock(mtx_);
        tasks_cv_.wait(lock, [this]() {
          return this->is_shutdown_ || !this->tasks_.empty();
        });

        if (is_shutdown_ && tasks_.empty()) {
          // is_shutdown_ 并且任务为空了之后才可以结束
          return;
        }

        free_thread_num_--;
        task = std::move(tasks_.front());
        tasks_.pop();
      }
      task();

      // 任务结束之后
      if (auto_grow_) {
        // 支持自动释放空闲线程,避免峰值过后大量空闲线程
        if (free_thread_num_ > 0 && workers_.size() > init_threads_num_) {
          return;
        }
      }

      {
        std::lock_guard<std::mutex> lock(mtx_);
        free_thread_num_++;
      }
    }
  }

  std::vector<std::thread> workers_;
  std::queue<TaskType> tasks_;

  std::mutex mtx_;                    // 任务队列互斥锁
  std::condition_variable tasks_cv_;  // 任务队列条件变量

  const size_t max_threads_num_;
  size_t init_threads_num_;
  std::atomic<size_t> free_thread_num_;  // 空闲线程数量

  std::atomic<bool> is_shutdown_{false};

  const bool auto_grow_;  // 是否支持自动扩缩容
  std::mutex grow_mtx_;   // 线程池增长互斥锁
};
**/
