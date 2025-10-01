cmake_minimum_required(VERSION 3.15)
project(capture_progress)

include_directories(include)

set(CMAKE_CXX_STANDARD 11)

add_library(capture_progress STATIC src/capture-progress/progress.cpp)