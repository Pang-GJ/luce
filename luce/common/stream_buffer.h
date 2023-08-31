#pragma once

#include <cstdint>
#include <vector>

class StreamBuffer : public std::vector<char> {
 public:
  StreamBuffer() : cur_pos_(0) {}
  StreamBuffer(const char *in, size_t len) : cur_pos_(0) {
    insert(begin(), in, in + len);
  }

  void reset() { cur_pos_ = 0; }

  const char *data() const { return &this->at(0); }
  const char *current() const { return &this->at(cur_pos_); }
  void offset(int k) { cur_pos_ += k; }

  bool is_eof() const { return cur_pos_ >= size(); }

  void input(char *in, size_t len) { insert(end(), in, in + len); }

  int findc(char c) {
    iterator iter = find(begin() + cur_pos_, end(), c);
    if (iter != end()) {
      return iter - (begin() + cur_pos_);
    }
    return -1;
  }

 private:
  uint32_t cur_pos_;
};