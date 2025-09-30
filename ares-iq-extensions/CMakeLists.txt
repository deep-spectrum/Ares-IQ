cmake_minimum_required(VERSION 3.15)
project(${SKBUILD_PROJECT_NAME}
        VERSION ${SKBUILD_PROJECT_VERSION}
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(extern/pybind11)

include(ExternalProject)
set(UHD_INSTALL_DIR ${CMAKE_BINARY_DIR}/uhd-install)
ExternalProject_Add(
        uhd
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/uhd/host
        BINARY_DIR ${CMAKE_BINARY_DIR}/uhd-build
        CMAKE_ARGS
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
         -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        BUILD_ALWAYS TRUE
        # INSTALL_COMMAND make install
)
list(APPEND CMAKE_PREFIX_PATH "${UHD_INSTALL_DIR}")

find_package(UHD REQUIRED)

include(UHDMinDepVersions)
set(UHD_BOOST_REQUIRED_COMPONENTS
        program_options
        system
        thread
)
include(UHDBoost)

include_directories(
        include
        ${Boost_INCLUDE_DIRS}
        ${UHD_INCLUDE_DIRS}
)
link_directories(${Boost_LIBRARY_DIRS})

# USRP module
python_add_library(_usrp MODULE src/usrp.cpp WITH_SOABI)
target_link_libraries(_usrp PRIVATE pybind11::headers ${UHD_LIBRARIES} ${Boost_LIBRARIES})
add_dependencies(_usrp uhd)
install(TARGETS _usrp DESTINATION usrp)

# Core module
python_add_library(_core MODULE src/core.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE pybind11::headers)
target_compile_definitions(_core PRIVATE VERSION_INFO=${PROJECT_VERSION})
install(TARGETS _core DESTINATION usrp)

