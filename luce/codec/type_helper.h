#pragma once

#include <deque>
#include <forward_list>
#include <list>
#include <utility>
#include <vector>
namespace codec {

class Serializer;

// 利用 SFINAE
// 即在模板实例化过程中，如果某个表达式的推导无效（例如，调用不存在的方法），模板不会被认为是错误，而是选择另一条候选路径。
// 通过使用decltype和std::declval来模拟方法调用，并使用SFINAE技术通过重载解析来进行检查。
// 注意，该方法仅适用于检查成员方法，无法用于检查非成员函数、静态方法或独立函数。
// 判断一个类有没有 'serialize' 方法
template <typename T>
struct HasSerializeMethod {
 private:
  template <typename C>
  static constexpr auto test(int)
      -> decltype(std::declval<C>().serialize(std::declval<Serializer *>()),
                  std::true_type());

  template <typename C>
  static constexpr auto test(...) -> decltype(std::false_type());

 public:
  static constexpr bool value =
      std::is_same_v<decltype(test<T>(0)), std::true_type>;
};

template <typename T>
constexpr bool has_serialize_method_v = HasSerializeMethod<T>::value;

// 判断一个类有没有 'deserialize' 方法
template <typename T>
struct HasDeserializeMethod {
 private:
  template <typename C>
  static constexpr auto test(int)
      -> decltype(std::declval<C>().deserialize(std::declval<Serializer *>()),
                  std::true_type());

  template <typename C>
  static constexpr auto test(...) -> decltype(std::false_type());

 public:
  static constexpr bool value =
      std::is_same_v<decltype(test<T>(0)), std::true_type>;
};

template <typename T>
constexpr bool has_deserialize_method_v = HasDeserializeMethod<T>::value;

// 判断一个类有没有 'size' 方法
template <typename T>
struct HasSizeMethod {
 private:
  template <typename C>
  static constexpr auto test(int)
      -> decltype(std::declval<C>().size(), std::true_type());

  template <typename C>
  static constexpr auto test(...) -> decltype(std::false_type());

 public:
  static constexpr bool value =
      std::is_same_v<decltype(test<T>(0)), std::true_type>;
};

template <typename T>
constexpr bool has_size_method_v = HasSizeMethod<T>::value;

// 默认类型为false
template <typename T>
struct IsSequenceContainerType {
  static constexpr bool value = false;
};

// Vector类型为true
template <typename T>
struct IsSequenceContainerType<std::vector<T>> {
  static constexpr bool value = true;
};

// deque类型
template <typename T>
struct IsSequenceContainerType<std::deque<T>> {
  static constexpr bool value = true;
};

// list 类型
template <typename T>
struct IsSequenceContainerType<std::list<T>> {
  static constexpr bool value = true;
};

// forward_list 类型
template <typename T>
struct IsSequenceContainerType<std::forward_list<T>> {
  static constexpr bool value = true;
};

// 定义获取容器类型的模板
template <typename T>
constexpr bool is_sequence_container_type_v = IsSequenceContainerType<T>::value;

}  // namespace codec