#independent env of test app 
cmake_minimum_required(VERSION 3.28)

project(little_test)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(test little_test.cpp)
