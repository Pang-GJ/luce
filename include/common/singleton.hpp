#pragma once

#include <cstdlib>
#include <utility>

template <class T>
class Singleton {
 public:
  static T &GetInstance() {
    static T instance;
    return instance;
  }

  Singleton(T &&) = delete;
  Singleton(const T &) = delete;
  Singleton &&operator=(const T &&) = delete;
  Singleton &operator=(const T &) = delete;

 protected:
  Singleton() = default;
  virtual ~Singleton() = default;
};

/**
template <class T>
class Singleton {
 public:
  Singleton() = delete;
  virtual ~Singleton() = delete;

  Singleton(const Singleton &) = delete;
  Singleton &operator=(const Singleton &) = delete;

  template <typename... Args>
  static T *instance(Args &&...args) {
    if (instance_ == nullptr) {
      instance_ = new T(std::forward<Args>(args)...);
    }
    return instance_;
  }

  static T *GetInstance() {
    if (instance_ == nullptr) {
      exit(-1);
    }
    return instance_;
  }

  static void DestroyInstance() {
    if (instance_) {
      delete instance_;
      instance_ = nullptr;
    }
  }

 private:
  static T *instance_;
};
**/
