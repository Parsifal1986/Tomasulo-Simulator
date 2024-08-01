cmake_minimum_required(VERSION 3.10)

project(Simulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories(Simu/include)
include_directories(Simu/module)

# add_executable(demo sample/main.cpp)

add_executable(code Simu/src/main.cpp)