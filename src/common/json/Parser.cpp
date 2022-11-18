#include "common/json/Parser.hpp"
#include <algorithm>
#include "common/json/JObject.hpp"

namespace tinyjson {

JObject Parser::FromString(string_view content) {
  static Parser instance;
  instance.Init(content);
  return instance.Parse();
}

JObject Parser::Parse() {
  char token = GetNextToken();
  if (token == 'n') {
    return ParseNull();
  }
  if (token == 't' || token == 'f') {
    return ParseBool();
  }
  if (token == '-' || (std::isdigit(token) != 0)) {
    return ParseNumber();
  }
  if (token == '\"') {
    return ParseString();
  }
  if (token == '[') {
    return ParseList();
  }
  if (token == '{') {
    return ParseDict();
  }

  throw std::logic_error("Unexpected character in parse json");
}

char Parser::GetNextToken() {
  while (std::isspace(str_[idx_]) != 0) {
    ++idx_;
  }

  if (idx_ >= str_.size()) {
    throw std::logic_error("Unexpected character in parse json");
  }
  // 如果是注释，需要跳过
  SkipComment();
  return str_[idx_];
}

JObject Parser::ParseNull() {
  if (str_.compare(idx_, 4, "null") == 0) {
    idx_ += 4;
    return {};
  }
  throw std::logic_error("parse null error");
}

JObject Parser::ParseBool() {
  if (str_.compare(idx_, 4, "true") == 0) {
    idx_ += 4;
    return JObject{true};
  }
  if (str_.compare(idx_, 5, "false") == 0) {
    idx_ += 5;
    return JObject{false};
  }
  throw std::logic_error("parse bool error");
}

JObject Parser::ParseNumber() {
  // 记录起始下标
  auto begin = idx_;
  // 处理整数部分
  if (isdigit(str_[idx_]) != 0) {
    while (isdigit(str_[idx_]) != 0) {
      ++idx_;
    }
  } else {
    throw std::logic_error("invalid character in number");
  }
  // 判断有没有小数部分，没有就直接返回
  if (str_[idx_] != '.') {
    return JObject{static_cast<int>(strtol(str_.c_str() + begin, nullptr, 10))};
  }
  // 处理小数部分
  if (str_[idx_] == '.') {
    idx_++;
    if (isdigit(str_[idx_]) == 0) {
      throw std::logic_error(
          "at least one digit required in parse float part!");
    }
    while (isdigit(str_[idx_]) != 0) {
      ++idx_;
    }
  }
  return JObject{strtof64(str_.c_str() + begin, nullptr)};
}

JObject Parser::ParseString() {
  auto pre_pos = ++idx_;
  auto pos = str_.find('"', idx_);
  if (pos != string::npos) {
    // 解析还没有结束，判断是否是转义符号
    while (true) {
      if (str_[pos - 1] != '\\') {  // 如果不是转义则结束解析
        break;
      }
      // 如果是转义字符，则判断是否已经被抵消，抵消完则跳出，否则继续寻找下个字符串结束符
      if (IsEscConsume(pos - 1)) {
        break;
      }

      pos = str_.find('"', pos + 1);
      if (pos == string::npos) {
        throw std::logic_error(R"(expected left '"' in parse string)");
      }
    }
    idx_ = pos + 1;
    return JObject{str_.substr(pre_pos, pos - pre_pos)};
  }
  throw std::logic_error("parse string error");
}

JObject Parser::ParseList() {
  // 得到list类型的JObject
  JObject arr((list_t()));
  ++idx_;

  char ch = GetNextToken();
  if (ch == ']') {
    ++idx_;
    return arr;
  }

  while (true) {
    arr.push_back(Parse());
    ch = GetNextToken();
    if (ch == ']') {
      ++idx_;
      break;
    }
    // 如果不是逗号
    if (ch != ',') {
      throw std::logic_error("expected ',' in parse list");
    }
    // 跳过逗号
    ++idx_;
  }

  return arr;
}

JObject Parser::ParseDict() {
  // 得到dict类型的JObject
  JObject dict((dict_t()));
  ++idx_;
  char ch = GetNextToken();
  if (ch == '}') {
    ++idx_;
    return dict;
  }

  while (true) {
    // 解析key
    string key = std::move(Parse().Value<string>());
    ch = GetNextToken();
    if (ch != ':') {
      throw std::logic_error("expected ':' in parse dict");
    }
    ++idx_;

    // 解析value
    dict[key] = Parse();
    ch = GetNextToken();
    if (ch == '}') {
      ++idx_;
      break;  // 解析完成
    }
    // 没有结束，要用逗号分隔
    if (ch != ',') {
      throw std::logic_error("expected ':' in parse dict");
    }
    // 跳过逗号
    ++idx_;
  }

  return dict;
}

void Parser::SkipComment() {
  // 跳过注释，标准json里是没有的，但VSCode的json会有注释
  if (str_.compare(idx_, 2, R"(//)") == 0) {
    while (true) {
      auto next_pos = str_.find('\n', idx_);
      if (next_pos == str_.size()) {
        throw std::logic_error("invalid comment area!");
      }
      // 查看下一行是否还是注释
      idx_ = next_pos + 1;
      while (isspace(str_[idx_]) != 0) {
        ++idx_;
      }
      if (str_.compare(idx_, 2, R"(//)") != 0) {
        // 注释结束了
        break;
      }
    }
  }
}

void Parser::Init(string_view src) {
  str_ = src;
  idx_ = 0;
  TrimRight();  // 去除右侧多余空格
}

void Parser::TrimRight() {
  // 去除尾部空字符，方便最后的结束检查
  str_.erase(std::find_if(str_.rbegin(), str_.rend(),
                          [](char ch) { return std::isspace(ch) == 0; })
                 .base(),
             str_.end());
}

bool Parser::IsEscConsume(size_t pos) {
  size_t end_pos = pos;
  while (str_[pos] == '\\') {
    --pos;
  }
  auto cnt = end_pos - pos;
  // 如果'\'的个数为偶数，则成功抵消，如果为奇数，则未抵消
  return cnt % 2 == 0;
}
}  // namespace tinyjson
