#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <utility>

#include "luce/common/stream_buffer.h"
namespace codec {

class Serializer {
 public:
  enum ByteOrder { BigEndian, LittleEndian };

  Serializer() : byteorder_(LittleEndian) {}
  ~Serializer() = default;
  explicit Serializer(StreamBuffer buf, ByteOrder byteorder = LittleEndian)
      : byteorder_(byteorder), stream_buf_(std::move(buf)) {}

  void reset() { stream_buf_.reset(); }

  size_t size() const { return stream_buf_.size(); }

  void skip_raw_data(int k) { stream_buf_.offset(k); }

  const char *data() const { return stream_buf_.data(); }

  void set_byte_order(char *in, int len) {
    if (byteorder_ == BigEndian) {
      std::reverse(in, in + len);
    }
  }

  void write_raw_data(char *in, int len) {
    stream_buf_.input(in, len);
    stream_buf_.offset(len);
  }

  const char *current() const { return stream_buf_.current(); }

  void clear() {
    stream_buf_.clear();
    reset();
  }

  template <typename T>
  void output_type(T &t);

  void output_type(std::string &in);

  template <typename T>
  void input_type(const T &t);

  void input_type(const std::string &in);

  void input_type(const char *in);

  void get_length_mem(char *p, int len) {
    std::memcpy(p, stream_buf_.current(), len);
    stream_buf_.offset(len);
  }

  template <typename Tuple, std::size_t id>
  void getv(Serializer &s, Tuple &t) {
    s >> std::get<id>(t);
  }

  template <typename Tuple, std::size_t... I>
  Tuple get_tuple(std::index_sequence<I...> index_sequence) {
    Tuple t;
    std::initializer_list<int>{((getv<Tuple, I>(*this, t)), 0)...};
    return t;
  }

  template <typename T>
  Serializer &operator>>(T &item) {
    output_type(item);
    return *this;
  }

  template <typename T>
  Serializer &operator<<(T item) {
    input_type(item);
    return *this;
  }

 private:
  ByteOrder byteorder_;
  StreamBuffer stream_buf_;
};

template <typename T>
inline void Serializer::output_type(T &t) {
  const auto len = sizeof(t);
  char *buffer = new char[len];
  if (!stream_buf_.is_eof()) {
    std::memcpy(buffer, stream_buf_.current(), len);
    stream_buf_.offset(len);
    set_byte_order(buffer, len);
    t = *reinterpret_cast<T *>(buffer);
  }
  delete[] buffer;
}

inline void Serializer::output_type(std::string &in) {
  const auto marklen = sizeof(uint16_t);
  char *buffer = new char[marklen];
  std::memcpy(buffer, stream_buf_.current(), marklen);
  stream_buf_.offset(marklen);
  set_byte_order(buffer, marklen);
  const auto len = *reinterpret_cast<uint16_t *>(buffer);
  delete[] buffer;
  if (len == 0) {
    return;
  }
  in.insert(in.begin(), stream_buf_.current(), stream_buf_.current() + len);
  stream_buf_.offset(len);
}

template <typename T>
inline void Serializer::input_type(const T &t) {
  const auto len = sizeof(t);
  char *buffer = new char[len];
  const char *p = reinterpret_cast<const char *>(&t);
  std::memcpy(buffer, p, len);
  set_byte_order(buffer, len);
  stream_buf_.input(buffer, len);
  delete[] buffer;
}

inline void Serializer::input_type(const std::string &in) {
  // 先存入字符串长度
  uint16_t len = in.size();
  char *p = reinterpret_cast<char *>(&len);
  set_byte_order(p, sizeof(uint16_t));
  stream_buf_.input(p, sizeof(uint16_t));

  if (len == 0) {
    return;
  }
  char *buffer = new char[len];
  std::memcpy(buffer, in.c_str(), len);
  stream_buf_.input(buffer, len);
  delete[] buffer;
}

inline void Serializer::input_type(const char *in) {
  input_type<std::string>(std::string(in));
}

}  // namespace codec