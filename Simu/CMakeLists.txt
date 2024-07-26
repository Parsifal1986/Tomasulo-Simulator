cmake_minimum_required(VERSION 3.10)

project(Simulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories(include)
include_directories(module)

add_executable(demo sample/main.cpp)

add_executable(main src/main.cpp)