#pragma once

#include <initializer_list>
#include <tuple>
#include <type_traits>
#include <utility>
#include "luce/codec/serializer.h"

namespace net::rpc {

template <typename T>
struct type_xx {
  using type = T;
};

template <>
struct type_xx<void> {
  using type = int8_t;
};

// 用tuple做参数调用函数模板类
template <typename Function, typename Tuple, std::size_t... Index>
decltype(auto) invoke_impl(Function&& func, Tuple&& t,
                           std::index_sequence<Index...> index_sequence) {
  return func(std::get<Index>(std::forward<Tuple>(t))...);
}

template <typename Function, typename Tuple>
decltype(auto) invoke(Function&& func, Tuple&& t) {
  constexpr auto size =
      std::tuple_size<typename std::decay<Tuple>::type>::value;
  return invoke_impl(std::forward<Function>(func), std::forward<Tuple>(t),
                     std::make_index_sequence<size>{});
}

// 调用帮助
template <typename R, typename F, typename ArgsTuple>
typename std::enable_if<std::is_same<R, void>::value,
                        typename type_xx<R>::type>::type
call_helper(F f, ArgsTuple args) {
  invoke(f, args);
  // return 0;
}

template <typename R, typename F, typename ArgsTuple>
typename std::enable_if<!std::is_same<R, void>::value,
                        typename type_xx<R>::type>::type
call_helper(F f, ArgsTuple args) {
  return invoke(f, args);
}

}  // namespace net::rpc