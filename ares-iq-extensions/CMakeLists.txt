cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME}
        VERSION ${SKBUILD_PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

set(ARES_IQ_UHD_PATH "${CMAKE_SOURCE_DIR}/extern/uhd/host")

if (NOT IS_DIRECTORY "${ARES_IQ_UHD_PATH}")
	message(FATAL_ERROR "Please run \"git submodule update --init\" before proceeding")
endif()

add_subdirectory(extern/pybind11)
add_subdirectory(capture_progress)

include(cmake/FindUHD.cmake)
include(cmake/AddPythonModule.cmake)

include_directories(
        include
		${Boost_INCLUDE_DIRS}
		${UHD_INCLUDE_DIRS}
)

# ares_iq_ext package
set(PACKAGE_NAME ares_iq_ext)
python_package(src/${PACKAGE_NAME} DESTINATION ${PACKAGE_NAME})
python_module(_usrp src/usrp/usrp.cpp
		DESTINATION ${PACKAGE_NAME}/usrp
		DEPENDENCIES uhd_static_lib uhd capture_progress
		LIBS capture_progress ${UHD_STATIC_LIB_LINK_FLAG} ${UHD_STATIC_LIB_DEPS}
)
python_module(_core src/core.cpp
		DESTINATION ${PACKAGE_NAME}
		DEFINITIONS VERSION_INFO=${PROJECT_VERSION}
)
