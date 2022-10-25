cmake_minimum_required(VERSION 3.14)

project(luce)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
    message(FATAL_ERROR "You cannot build in a source directory
  (or any directory with a CMakeLists.txt file).
  Please make a build subdirectory.
  Feel free to remove CMakeCache.txt and CMakeFiles."
            )
endif()

# For clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build test executable" OFF)

# add ccache
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    message(use ccache to speed compile!)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

set(LIBRARY_NAME luce)

# 3rdparty dir:
set(PROJECT_TRDPARTY_DIR ${PROJECT_SOURCE_DIR}/3rdparty)

add_subdirectory(src)

if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_TESTS)

add_subdirectory(example)

# cmake module
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(FormatAndTidy)