cmake_minimum_required(VERSION 3.15)
project(logging)

set(CMAKE_CXX_STANDARD 11)

add_library(logging STATIC src/logger.cpp)
set_target_properties(logging PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
