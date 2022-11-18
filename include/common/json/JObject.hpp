#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace tinyjson {

using std::get_if;
using std::map;
using std::string;
using std::string_view;
using std::variant;
using std::vector;

enum TYPE { T_NULL, T_BOOL, T_INT, T_DOUBLE, T_STR, T_LIST, T_DICT };

class JObject;

using null_t = string;
using int_t = int32_t;
using bool_t = bool;
using double_t = double;
using str_t = string;
using list_t = vector<JObject>;
using dict_t = map<string, JObject>;

#define IS_TYPE(typea, typeb) std::is_same<typea, typeb>::value

template <class T>
constexpr bool IsBasicType() {
  return static_cast<bool>(IS_TYPE(T, str_t) || IS_TYPE(T, bool_t) ||
                           IS_TYPE(T, double_t) || IS_TYPE(T, int_t));
}

class JObject {
 public:
  using value_t = variant<bool_t, int_t, double_t, str_t, list_t, dict_t>;
  JObject() {
    type_ = T_NULL;
    value_ = "null";
  }

  explicit JObject(int_t value) { Int(value); }

  explicit JObject(bool_t value) { Bool(value); }

  explicit JObject(double_t value) { Double(value); }

  explicit JObject(const str_t &value) { Str(value); }

  explicit JObject(list_t value) { List(std::move(value)); }

  explicit JObject(dict_t value) { Dict(std::move(value)); }

  void Null() {
    type_ = T_NULL;
    value_ = "null";
  }

  void Int(int_t value) {
    type_ = T_INT;
    value_ = value;
  }

  void Bool(bool_t value) {
    type_ = T_BOOL;
    value_ = value;
  }

  void Double(double_t value) {
    type_ = T_DOUBLE;
    value_ = value;
  }

  void Str(string_view value) {
    type_ = T_STR;
    value_ = string(value);
  }

  void List(list_t value) {
    type_ = T_LIST;
    value_ = std::move(value);
  }

  void Dict(dict_t value) {
    type_ = T_DICT;
    value_ = std::move(value);
  }

  string ToString();

  JObject &operator[](const string &key) {
    if (type_ == T_DICT) {
      auto &dict = Value<dict_t>();
      return dict[key];
    }
    throw std::logic_error("not dict type! JObject::operator[]");
  }

  void push_back(JObject item) {
    if (type_ == T_LIST) {
      auto &list = Value<list_t>();
      list.push_back(std::move(item));
      return;
    }
    throw std::logic_error("not list type! JObject::push_back()");
  }

  void pop_back() {
    if (type_ == T_LIST) {
      auto &list = Value<list_t>();
      list.pop_back();
      return;
    }
    throw std::logic_error("not list type! JObject::push_back()");
  }

#define THROW_GET_ERROR(type) \
  throw std::logic_error("type error in get " #type " value!")

  template <class V>
  V &Value() {
    // 添加安全检查
    if constexpr (IS_TYPE(V, str_t)) {
      if (type_ != T_STR) {
        THROW_GET_ERROR(string);
      }
    } else if constexpr (IS_TYPE(V, int_t)) {
      if (type_ != T_INT) {
        THROW_GET_ERROR(INT);
      }
    } else if constexpr (IS_TYPE(V, bool_t)) {
      if (type_ != T_BOOL) {
        THROW_GET_ERROR(BOOL);
      }
    } else if constexpr (IS_TYPE(V, double_t)) {
      if (type_ != T_DOUBLE) {
        THROW_GET_ERROR(DOUBLE);
      }
    } else if constexpr (IS_TYPE(V, list_t)) {
      if (type_ != T_LIST) {
        THROW_GET_ERROR(LIST);
      }
    } else if constexpr (IS_TYPE(V, dict_t)) {
      if (type_ != T_DICT) {
        THROW_GET_ERROR(DICT);
      }
    }

    void *v = Value();
    if (v == nullptr) {
      throw std::logic_error("unknown type in JObject::Value()");
    }
    return *static_cast<V *>(v);
  }

  TYPE Type() { return type_; }

 private:
  void *Value();

  TYPE type_;
  value_t value_;
};

}  // namespace tinyjson
