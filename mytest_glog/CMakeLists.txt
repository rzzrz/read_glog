cmake_minimum_required (VERSION 3.16)
project (myproj VERSION 1.0)


set(CMAKE_BUILD_TYPE Debug)
set(CXX_STANDARD_REQUIRED true)
set(CXX_STANDARD 17)

find_package (glog 0.8.0 REQUIRED)

add_executable (main main.cpp)
target_link_libraries (main glog::glog)