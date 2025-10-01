cmake_minimum_required(VERSION 3.15)
project(capture_progress)

set(CMAKE_CXX_STANDARD 11)

add_library(capture_progress STATIC src/capture-progress/progress.cpp)
set_target_properties(capture_progress PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(capture_progress PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
