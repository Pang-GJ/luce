#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <forward_list>
#include <initializer_list>
#include <iterator>
#include <map>
#include <type_traits>
#include <utility>
#include <vector>

#include "luce/codec/type_helper.h"

namespace codec {

class Serializer {
 public:
  Serializer() = default;
  explicit Serializer(const std::string& data)
      : buffer_(data.begin(), data.end()) {}

  using IterType = std::vector<char>::iterator;
  using ConstIterType = std::vector<char>::const_iterator;

  Serializer(IterType begin, IterType end) : buffer_(begin, end) {}
  Serializer(ConstIterType begin, ConstIterType end) : buffer_(begin, end) {}

  ConstIterType cbegin() const { return buffer_.cbegin(); }
  ConstIterType cend() const { return buffer_.cend(); }

  template <typename T>
  std::enable_if_t<std::is_arithmetic_v<T>, void> serialize(const T& value) {
    const char* data = reinterpret_cast<const char*>(&value);
    buffer_.insert(buffer_.end(), data, data + sizeof(value));
  }

  template <typename T>
  std::enable_if_t<std::is_arithmetic_v<T>, void> deserialize(T* value) {
    char* data = reinterpret_cast<char*>(value);
    const auto size = sizeof(*value);
    std::copy(buffer_.data() + deserialize_pos_,
              buffer_.data() + deserialize_pos_ + size, data);
    deserialize_pos_ += size;
  }

  template <typename T>
  std::enable_if_t<has_serialize_method_v<T>, void> serialize(const T& value) {
    value.serialize(this);
  }

  template <typename T>
  std::enable_if_t<has_deserialize_method_v<T>, void> deserialize(T* value) {
    value->deserialize(this);
  }

  void serialize(const std::string& value) {
    const auto size = value.size();
    serialize(size);
    buffer_.insert(buffer_.end(), value.begin(), value.end());
  }

  void deserialize(std::string* value) {
    auto size = value->size();
    deserialize(&size);
    value->resize(size);
    std::copy(buffer_.data() + deserialize_pos_,
              buffer_.data() + deserialize_pos_ + size, value->begin());
    deserialize_pos_ += size;
  }

  // // std::vector 的序列化和反序列化
  // template <typename T>
  // void serialize(const std::vector<T> &value) {
  //   const auto size = value.size();
  //   serialize(size);
  //   std::for_each(value.cbegin(), value.cend(),
  //                 [&](const T &item) { serialize(item); });
  // }

  // template <typename T>
  // void deserialize(std::vector<T> *value) {
  //   auto size = value->size();
  //   deserialize(&size);
  //   value->resize(size);
  //   std::for_each(value->begin(), value->end(),
  //                 [&](T &item) { deserialize(&item); });
  // }

  // 顺序容器的序列化和反序列化
  template <typename Container>
  std::enable_if_t<is_sequence_container_type_v<Container>, void> serialize(
      const Container& value) {
    uint32_t size = 0;
    if constexpr (has_size_method_v<Container>) {
      size = value.size();
    } else {
      for (auto iter = value.cbegin(); iter != value.cend(); ++iter) {
        ++size;
      }
    }
    serialize(size);
    std::for_each(
        value.cbegin(), value.cend(),
        [&](const typename Container::value_type& item) { serialize(item); });
  }

  template <typename Container>
  std::enable_if_t<is_sequence_container_type_v<Container>, void> deserialize(
      Container* value) {
    uint32_t size = 0;
    deserialize(&size);
    value->resize(size);
    std::for_each(
        value->begin(), value->end(),
        [&](typename Container::value_type& item) { deserialize(&item); });
  }

  // std::forward_list: (为什么没有size方法.....)
  // template <typename T>
  // void serialize(const std::forward_list<T> &value) {
  //   std::list<T> tmp{value.cbegin(), value.cend()};
  //   serialize(tmp);
  // }

  // template <typename T>
  // void deserialize(std::forward_list<T> *value) {
  //   std::list<T> tmp;
  //   deserialize(&tmp);
  //   value->assign(tmp.begin(), tmp.end());
  // }

  // std::map 的序列化和反序列化
  template <typename Key, typename Value>
  void serialize(const std::map<Key, Value>& value) {
    const auto size = value.size();
    serialize(size);
    for (const auto& [key, val] : value) {
      serialize(key);
      serialize(val);
    }
  }

  template <typename Key, typename Value>
  void deserialize(std::map<Key, Value>* value) {
    auto size = value->size();
    deserialize(&size);
    for (auto i = 0; i < size; ++i) {
      Key key;
      Value val;
      deserialize(key);
      deserialize(val);
      (*value)[key] = val;
    }
  }

  // std::tuple 的序列化和反序列化
  template <typename... Args>
  void serialize(const std::tuple<Args...>& value) {
    std::apply([&](const Args&... args) { (serialize(args), ...); }, value);
  }

  template <typename... Args>
  void deserialize(std::tuple<Args...>* value) {
    std::apply([&](Args&... args) { (deserialize(&args), ...); }, *value);
  }

  std::string str() const { return {buffer_.begin(), buffer_.end()}; }

  size_t size() const { return buffer_.size(); }

  void clear() {
    buffer_.clear();
    deserialize_pos_ = 0;
  }

 private:
  std::vector<char> buffer_;
  int deserialize_pos_ = 0;
};

}  // namespace codec