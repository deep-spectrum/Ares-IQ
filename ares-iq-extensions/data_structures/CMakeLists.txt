cmake_minimum_required(VERSION 3.15)
project(ares_data_structures)

set(CMAKE_CXX_STANDARD 11)

add_library(ares_data_structures STATIC src/ares/queue.cpp)
set_target_properties(ares_data_structures PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(ares_data_structures PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
