#pragma once

#include <sstream>
#include <string>
#include <string_view>

#include "common/json/JObject.hpp"

namespace tinyjson {

using std::string;
using std::string_view;
using std::stringstream;

#define FUNC_TO_NAME _to_json
#define FUNC_FROM_NAME _from_json

#define START_TO_JSON void FUNC_TO_NAME(tinyjson::JObject &obj) const {
#define to(key) obj[key]
// push 一个自定义类型成员
#define to_struct(key, struct_member)            \
  do {                                           \
    tinyjson::JObject tmp((tinyjson::dict_t())); \
    (struct_member).FUNC_TO_NAME(tmp);           \
    obj[key] = tmp;                              \
  } while (0)
#define END_TO_JSON }

#define START_FROM_JSON void FUNC_FROM_NAME(tinyjson::JObject &obj) {
#define from(key, type) obj[key].Value<type>()
#define from_struct(key, struct_member) struct_member.FUNC_FROM_NAME(obj[key])
#define END_FROM_JSON }

class Parser {
 public:
  Parser() = default;

  static JObject FromString(string_view content);

  template <class T>
  static string ToJSON(const T &src) {
    // 如果是基本类型
    if constexpr (IS_TYPE(T, int_t) || IS_TYPE(T, bool_t) ||
                  IS_TYPE(T, double_t) || IS_TYPE(T, str_t)) {
      JObject object(src);
      return object.ToString();
    }
    // 如果是自定义类型，调用方法完成dict的赋值，然后to_string即可
    JObject obj((dict_t()));
    src.FUNC_TO_NAME(obj);
    return obj.ToString();
  }

  template <class T>
  static T FromJson(string_view src) {
    JObject obj = FromString(src);
    // 如果是基本类型
    if constexpr (IsBasicType<T>()) {
      return obj.template Value<T>();  // 疑问：加不加template有区别吗？？
    }

    if (obj.Type() != T_DICT) {
      throw std::logic_error("not dict type fromjson");
    }
    T ret;
    ret.FUNC_FROM_NAME(obj);
    return ret;
  }

  void Init(string_view src);
  char GetNextToken();
  JObject Parse();
  JObject ParseNull();
  JObject ParseBool();
  JObject ParseNumber();
  JObject ParseString();
  JObject ParseList();
  JObject ParseDict();

 private:
  string str_;
  size_t idx_{};

  void SkipComment();

  void TrimRight();

  bool IsEscConsume(size_t pos);
};

}  // namespace tinyjson
