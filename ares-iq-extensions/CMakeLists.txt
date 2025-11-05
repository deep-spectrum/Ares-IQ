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

# Signal Hound module
set(SIGNAL_HOUND_LIBS
		${CMAKE_SOURCE_DIR}/lib/libbb_api.so.5
		${CMAKE_SOURCE_DIR}/lib/libftd2xx.so
)

python_add_library(_signal_hound MODULE src/signal-hound/bb_device.cpp WITH_SOABI)
add_dependencies(_signal_hound capture_progress)
target_link_libraries(_signal_hound PRIVATE pybind11::headers capture_progress ${SIGNAL_HOUND_LIBS})
install(TARGETS _signal_hound DESTINATION ares_iq_ext/signal_hound)
install(FILES src/ares_iq_ext/signal-hound/__init__.py DESTINATION ares_iq_ext/signal_hound)
install(FILES
		lib/libbb_api.so.5
		lib/libftd2xx.so
		DESTINATION ares_iq_ext/signal_hound/lib)
if(APPLE)
	set(origin_token "@loader_path")
else()
	set(origin_token "$ORIGIN")
endif()
set_property(TARGET _signal_hound PROPERTY
		INSTALL_RPATH "${origin_token}/lib"
)
