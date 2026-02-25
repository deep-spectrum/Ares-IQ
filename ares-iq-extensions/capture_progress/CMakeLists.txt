cmake_minimum_required(VERSION 3.15)
project(capture_progress)

set(CMAKE_CXX_STANDARD 17)

option(USE_PYTHON_LIB "Use Python library implementation of CaptureProgress" OFF)

add_library(capture_progress STATIC src/capture-progress/progress.cpp src/capture-progress/monitor.cpp src/capture-progress/rich_display.cpp)
set_target_properties(capture_progress PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(capture_progress PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (USE_PYTHON_LIB)
    find_package(pybind11 QUIET)

    if (TARGET pybind11::embed)
        message(STATUS "Found pybind11::embed")
        target_link_libraries(capture_progress PRIVATE pybind11::embed)
        target_compile_definitions(capture_progress PRIVATE USE_PYTHON_LIB)
    else()
        message(WARNING "USE_PYTHON_LIB is ON but pybind11::embed is not available")
    endif()
endif()
