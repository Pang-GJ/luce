cmake_minimum_required(VERSION 3.20)

project(luce)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# mymuduo最终编译成动态库，设置动态库路径,放在根目录的lib目录下面
set(LIBRARY_OUTPUT_PATH, ${CMAKE_CURRENT_LIST_DIR}/lib)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_STANDARD 17)
# 设置调试信息
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fPIC")
add_compile_options(-g -Wall -fPIC)

# For Debug
add_definitions(-DMUDEBUG)

# 定义参与编译的源文件
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_LIST) # .代表所有源文件

# 编译生成动态库
add_library(luce SHARED ${SRC_LIST})

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
