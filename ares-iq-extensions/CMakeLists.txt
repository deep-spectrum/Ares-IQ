cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME}
        VERSION ${SKBUILD_PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS )

set(ARES_IQ_UHD_PATH "${CMAKE_SOURCE_DIR}/extern/uhd/host")

if (NOT IS_DIRECTORY "${ARES_IQ_UHD_PATH}")
	message(FATAL_ERROR "Please run \"git submodule update --init\" before proceeding")
endif()

add_subdirectory(extern/pybind11)
add_subdirectory(capture_progress)

set(UHD_INSTALL_DIR $ENV{HOME}/.ares-iq/uhd)
file(GLOB UHD_LIB_SO "${UHD_INSTALL_DIR}/lib/*.so.*")
set(UHD_LIB_PATH ${UHD_INSTALL_DIR}/lib/libuhd.a)

if (NOT EXISTS ${UHD_LIB_PATH})
	include(ExternalProject)
	ExternalProject_Add(
    	    uhd
        	SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/uhd/host
	        BINARY_DIR ${CMAKE_BINARY_DIR}/uhd-build
			CONFIGURE_COMMAND
				${CMAKE_COMMAND}
   		      		-DCMAKE_INSTALL_PREFIX=${UHD_INSTALL_DIR}
        	 		-DENABLE_PYTHON_API=OFF
         			-DENABLE_LIBUHD=ON
         			-DENABLE_TESTS=OFF
         			-DENABLE_UTILS=OFF
         			-DENABLE_EXAMPLES=OFF
         			-DENABLE_USB=OFF
         			-DENABLE_MANUAL=OFF
         			-DENABLE_DOXYGEN=OFF
         			-DENABLE_MAN_PAGES=OFF
					-DENABLE_STATIC_LIBS=ON
         			-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
					${CMAKE_SOURCE_DIR}/extern/uhd/host
        	BUILD_COMMAND
				${CMAKE_MAKE_PROGRAM}
			INSTALL_COMMAND
				${CMAKE_MAKE_PROGRAM} install
	)
	add_custom_target(import_uhd ALL
		DEPENDS uhd
		COMMAND ${CMAKE_COMMAND} -E echo "UHD built and installed to ${UHD_INSTALL_DIR}"
	)

	add_custom_command(
			OUTPUT ${UHD_LIB_PATH}
			COMMAND ${CMAKE_COMMAND} -E echo "Waiting for static library"
			DEPENDS uhd
	)
else()
	message(STATUS "Found ${UHD_LIB_PATH}")
endif()

add_custom_target(uhd_static_lib DEPENDS ${UHD_LIB_PATH})

list(APPEND CMAKE_PREFIX_PATH "${UHD_INSTALL_DIR}")

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS
		chrono
		date_time
		filesystem
		program_options
		serialization
		system
		thread
)
# message(FATAL_ERROR "${Boost_LIBRARIES} ${Boost_LIBRARY_DIR}")

include_directories(
        include
		${Boost_INCLUDE_DIRS}
		${UHD_INSTALL_DIR}/include
)
link_directories(${Boost_LIBRARY_DIRS})
set(UHD_STATIC_FLAGS "-Wl,-whole-archive ${UHD_LIB_PATH} -Wl,-no-whole-archive")
set(UHD_STATIC_DEPS "dl;pthread;boost_chrono;boost_date_time;boost_filesystem;boost_program_options;boost_serialization;boost_thread")

# USRP module
python_add_library(_usrp MODULE src/usrp/usrp.cpp WITH_SOABI)
add_dependencies(_usrp uhd_static_lib capture_progress)
target_link_libraries(_usrp PRIVATE pybind11::headers capture_progress ${UHD_STATIC_FLAGS} ${UHD_STATIC_DEPS})
install(TARGETS _usrp DESTINATION ares_iq_ext/usrp)
install(FILES src/ares_iq_ext/usrp/__init__.py DESTINATION ares_iq_ext/usrp)

# Core module
python_add_library(_core MODULE src/core.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers)
target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})
install(TARGETS _core DESTINATION ares_iq_ext)
install(FILES src/ares_iq_ext/__init__.py DESTINATION ares_iq_ext)
