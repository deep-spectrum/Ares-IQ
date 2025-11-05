cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME}
        VERSION ${SKBUILD_PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

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

include(FindUHD)
include(CheckLinuxDistro)

if (${LINUX_DISTRO} STREQUAL "ubuntu")
	if (${LINUX_VERSION_MAJOR} LESS ${MIN_UBUNTU_VERSION} OR ${LINUX_VERSION_MAJOR} GREATER ${MAX_UBUNTU_VERSION})
		message(FATAL_ERROR "Ubuntu version not supported. Ubuntu is supported from version ${MIN_UBUNTU_VERSION} to ${MAX_UBUNTU_VERSION}")
	endif()
	set(SM_API_LIBS "${CMAKE_SOURCE_DIR}/lib/sm_api/Ubuntu/libsm_api.so.2")
else()
	message(FATAL_ERROR "${LINUX_DISTRO} is not supported")
endif()

include_directories(
        include
		${Boost_INCLUDE_DIRS}
		${UHD_INCLUDE_DIRS}
)

# USRP module
python_add_library(_usrp MODULE src/usrp/usrp.cpp WITH_SOABI)
add_dependencies(_usrp uhd_static_lib uhd capture_progress)
target_link_libraries(_usrp PRIVATE pybind11::headers capture_progress ${UHD_STATIC_LIB_LINK_FLAG} ${UHD_STATIC_LIB_DEPS})
install(TARGETS _usrp DESTINATION ares_iq_ext/usrp)
install(FILES src/ares_iq_ext/usrp/__init__.py DESTINATION ares_iq_ext/usrp)

# Core module
python_add_library(_core MODULE src/core.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers)
target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})
install(TARGETS _core DESTINATION ares_iq_ext)
install(FILES src/ares_iq_ext/__init__.py DESTINATION ares_iq_ext)

# Signal Hound SM series
set(SIGNAL_HOUND_INSTALL ares_iq_ext/signal_hound)
set(SIGNAL_HOUND_INSTALL_LIBS ${SIGNAL_HOUND_INSTALL}/lib)
python_add_library(_sh_sm_series MODULE src/signal-hound/sm_series.cpp WITH_SOABI)
add_dependencies(_sh_sm_series capture_progress)
target_link_libraries(_sh_sm_series PRIVATE pybind11::headers capture_progress ${SM_API_LIBS})
if (UNIX)
	if (APPLE)
		set(origin_token "@loader_path")
	else ()
		set(origin_token "$ORIGIN")
	endif ()
	set_property(TARGET _sh_sm_series PROPERTY
			INSTALL_RPATH "${origin_token}/lib"
	)
endif ()
install(TARGETS _sh_sm_series DESTINATION ${SIGNAL_HOUND_INSTALL})
install(FILES src/ares_iq_ext/signal-hound/__init__.py DESTINATION ${SIGNAL_HOUND_INSTALL})
install(FILES ${SM_API_LIBS} DESTINATION ${SIGNAL_HOUND_INSTALL_LIBS})
