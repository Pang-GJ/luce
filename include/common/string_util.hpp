#pragma once

// learn from https://github.com/Tencent/flare/blob/master/flare/base/string.cc

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>

std::string Join(const std::vector<std::string_view> &parts,
                 std::string_view delim);

std::string Join(const std::vector<std::string> &parts, std::string_view delim);

std::string Join(const std::initializer_list<std::string_view> &parts,
                 std::string_view delim);

bool StartsWith(std::string_view str, std::string_view prefix);

bool EndsWith(std::string_view str, std::string_view suffix);

void Replace(std::string &target, std::string_view from, std::string_view to,
             std::size_t count);

std::string Replace(std::string_view target, std::string_view from,
                    std::string_view to, std::size_t count);

std::string_view Trim(std::string_view str);

std::vector<std::string_view> Split(std::string_view str,
                                    std::string_view delim,
                                    bool keep_empty = true);

std::vector<std::string_view> Split(std::string_view str, char delim,
                                    bool keep_empty = true);

bool Contains(std::string_view str, std::string_view target);

// std::format in linux could not use now
// use fmtlib instead
template <typename... Args>
std::string Format(std::string_view real_time_fmt, Args &&...args) {
  return fmt::vformat(real_time_fmt, fmt::make_format_args(args...));
}
