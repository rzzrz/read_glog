cmake_minimum_required (VERSION 3.16)
project (myproj VERSION 1.0)


set(CMAKE_BUILD_TYPE Debug)
set(CXX_STANDARD_REQUIRED true)
set(CXX_STANDARD 17)

find_package (glog 0.8.0 REQUIRED)

#添加宏的debug信息
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-g3 -O0)  # -g3 保留宏信息，-O0 禁用优化
endif()

add_executable (main main.cpp)
target_link_libraries (main glog::glog)