cmake_minimum_required(VERSION 3.20)
project(breathing_led CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译警告，帮助提前发现潜在隐患
add_compile_options(-Wall -Wextra -Wpedantic -g)

add_executable(breathing_led src/main.cpp)

# 告知编译器头文件搜索路径（对应 include 文件夹）
target_include_directories(breathing_led PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

