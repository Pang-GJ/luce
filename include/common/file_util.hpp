#pragma once

#include <dirent.h>
#include <sys/stat.h>
#include <string_view>

size_t FileSize(std::string_view path) {
  struct stat info {};
  stat(path.data(), &info);
  return info.st_size;
}
