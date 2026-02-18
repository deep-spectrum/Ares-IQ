cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME}
        VERSION ${SKBUILD_PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")  # or 11, 14, 17, 20
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # optional, ensure standard is supported
set(CMAKE_CXX_EXTENSIONS OFF)  # optional, keep compiler extensions off

set(MIN_UBUNTU_VERSION "18")
set(MAX_UBUNTU_VERSION "24")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(ARES_IQ_UHD_PATH "${CMAKE_SOURCE_DIR}/extern/uhd/host")
set(ERROR_NOT_LINUX TRUE)

if (NOT IS_DIRECTORY "${ARES_IQ_UHD_PATH}")
	message(FATAL_ERROR "Please run \"git submodule update --init\" before proceeding")
endif()

add_subdirectory(extern/pybind11)
add_subdirectory(capture_progress)
add_subdirectory(logging)
add_subdirectory(data_structures)

include(FindUHD)
include(CheckLinuxDistro)
include(AddPythonModule)

if (${LINUX_DISTRO} STREQUAL "ubuntu")
	if (${LINUX_VERSION_MAJOR} LESS ${MIN_UBUNTU_VERSION} OR ${LINUX_VERSION_MAJOR} GREATER ${MAX_UBUNTU_VERSION})
		message(FATAL_ERROR "Ubuntu version not supported. Ubuntu is supported from version ${MIN_UBUNTU_VERSION} to ${MAX_UBUNTU_VERSION}")
	endif()
	set(SM_API_LIBS "${CMAKE_SOURCE_DIR}/lib/signal-hound/sm_api/Ubuntu/libsm_api.so.2")
else()
	message(FATAL_ERROR "${LINUX_DISTRO} is not supported")
endif()

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
python_module(_sh_sm_series src/signal-hound/sm_series.cpp
		DESTINATION ${PACKAGE_NAME}/signal_hound
		DEPENDENCIES capture_progress logging ares_data_structures
		LIBS capture_progress ${SM_API_LIBS} logging ares_data_structures
		INSTALL_LIBS ${SM_API_LIBS}
)
