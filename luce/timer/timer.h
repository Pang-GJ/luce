#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace timer {
using TimePoint = std::chrono::system_clock::time_point;
using TimerTask = std::function<void()>;

struct Timer {
  Timer() = default;
  Timer(TimePoint expiry_time, size_t id, TimerTask task)
      : expiry_time(expiry_time), timer_id(id), task(std::move(task)) {}
  TimePoint expiry_time;
  size_t timer_id;
  TimerTask task;
};

// A simple timer manager
class TimerManager {
 public:
  size_t AddTimer(uint32_t milliseconds, const TimerTask& callback) {
    TimePoint now = std::chrono::system_clock::now();
    TimePoint expiry_time = now + std::chrono::milliseconds(milliseconds);
    std::unique_lock lock(mtx_);
    auto id = timer_id_++;
    Push(Timer{expiry_time, id, callback});
    return id;
  }

  void RemoveTimer(size_t timer_id) {
    std::unique_lock lock(mtx_);
    auto iter = std::find_if(
        timers_.begin(), timers_.end(),
        [&](const Timer& timer) { return timer.timer_id == timer_id; });
    if (iter != timers_.end()) {
      Erase(iter - timers_.begin());
    }
  }

  void Tick() {
    while (!stop_) {
      auto now = std::chrono::system_clock::now();
      std::unique_lock lock(mtx_);
      if (!Empty() && now >= top().expiry_time) {
        auto timer = pop();
        std::thread(timer.task).detach();
      } else {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

  void Shutdown() { stop_ = true; }

 private:
  void HeapifyUp(uint32_t index) {
    if (index == 0) {
      return;
    }
    uint32_t parent_index = (index - 1) / 2;
    if (timers_[index].expiry_time < timers_[parent_index].expiry_time) {
      std::swap(timers_[index], timers_[parent_index]);
      HeapifyUp(parent_index);
    }
  }

  void HeapifyDown(uint32_t index) {
    uint32_t left_child_index = index * 2 + 1;
    uint32_t right_child_index = index * 2 + 2;
    uint32_t smallest_index = index;
    if (left_child_index < timers_.size() &&
        timers_[left_child_index].expiry_time <
            timers_[smallest_index].expiry_time) {
      smallest_index = left_child_index;
    }
    if (right_child_index < timers_.size() &&
        timers_[right_child_index].expiry_time <
            timers_[smallest_index].expiry_time) {
      smallest_index = right_child_index;
    }
    if (smallest_index != index) {
      std::swap(timers_[smallest_index], timers_[index]);
      HeapifyDown(smallest_index);
    }
  }

  void Erase(uint32_t index) {
    std::swap(timers_[index], timers_.back());
    timers_.pop_back();
    HeapifyDown(index);
  }

  bool Empty() const { return timers_.empty(); }

  Timer& top() { return timers_.front(); }

  Timer pop() {
    if (timers_.empty()) {
      return {};
    }
    auto& timer = top();
    Erase(0);
    return timer;
  }

  void Push(const Timer& timer) {
    timers_.emplace_back(timer);
    HeapifyUp(timers_.size() - 1);
  }

  std::vector<Timer> timers_;  // heap
  static size_t timer_id_;
  std::atomic<bool> stop_{false};
  std::mutex mtx_;
};

}  // namespace timer