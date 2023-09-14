#include "luce/common/string_util.h"

namespace String {

template <typename T>
void JoinImpl(const T& parts, std::string_view delim, std::string* result) {
  size_t size = 0;
  for (auto&& item : parts) {
    size += item.size() + delim.size();
  }
  result->clear();
  if (size == 0) {
    return;
  }
  size -= delim.size();  // 去除最后一个delim
  result->reserve(size);

  for (auto it = parts.begin(); it != parts.end(); ++it) {
    if (it != parts.begin()) {
      result->append(delim.begin(), delim.end());
    }
    result->append(it->begin(), it->end());
  }
}

std::string Join(const std::vector<std::string_view>& parts,
                 std::string_view delim) {
  std::string result;
  JoinImpl(parts, delim, &result);
  return result;
}

std::string Join(const std::vector<std::string>& parts,
                 std::string_view delim) {
  std::string result;
  JoinImpl(parts, delim, &result);
  return result;
}

std::string Join(const std::initializer_list<std::string_view>& parts,
                 std::string_view delim) {
  std::string result;
  JoinImpl(parts, delim, &result);
  return result;
}

bool StartsWith(std::string_view str, std::string_view prefix) {
  return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

bool EndsWith(std::string_view str, std::string_view suffix) {
  return str.size() >= suffix.size() &&
         str.substr(str.size() - suffix.size()) == suffix;
}

void Replace(std::string& target, std::string_view from, std::string_view to,
             std::size_t count) {
  auto p = target.find(from);
  while (p != std::string::npos && ((count--) != 0U)) {
    target.replace(p, from.size(), to);
    p = target.find(from, p + to.size());
  }
}

std::string Replace(std::string_view target, std::string_view from,
                    std::string_view to, std::size_t count) {
  std::string tmp(target);
  Replace(tmp, from, to, count);
  return tmp;
}

std::string_view Trim(std::string_view str) {
  std::size_t start = 0;
  std::size_t end = str.size();
  if (str.empty()) {
    return {};
  }
  if (str.front() == ' ') {
    start = str.find_first_not_of(' ');
  }
  if (start == std::string_view::npos) {
    return {};
  }
  if (str.back() == ' ') {
    end = str.find_last_not_of(' ');
  }
  return str.substr(start, end - start + 1);
}

std::vector<std::string_view> Split(std::string_view str,
                                    std::string_view delim, bool keep_empty) {
  std::vector<std::string_view> result;
  if (str.empty()) {
    return result;
  }
  auto current = str;
  while (true) {
    auto pos = current.find(delim);
    if (pos != 0 || keep_empty) {
      result.emplace_back(current.substr(0, pos));
    }
    if (pos == std::string_view::npos) {
      break;
    }
    current = current.substr(pos + delim.size());
    if (current.empty()) {
      if (keep_empty) {
        result.emplace_back("");
      }
      break;
    }
  }
  return result;
}

std::vector<std::string_view> Split(std::string_view str, char delim,
                                    bool keep_empty) {
  return Split(str, std::string_view(&delim, 1), keep_empty);
}

bool Contains(std::string_view str, std::string_view target) {
  if (str == target) {
    return true;
  }
  if (str.empty()) {
    return false;
  }
  if (target.empty()) {
    return true;
  }
  auto pos = str.find(target);
  return pos != std::string_view::npos;
}

}  // namespace String
