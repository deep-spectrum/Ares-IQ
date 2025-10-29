set(_UHD_DIR "$ENV{HOME}/.ares-iq/uhd")
set(UHD_DIR ${_UHD_DIR})
set(ENV{UHD_DIR} ${UHD_DIR})
set(_UHD_STATIC_LIB "${UHD_DIR}/lib/libuhd.a")

option(UHD_USE_STATIC_LIBS ON)
find_package(UHD QUIET PATHS ${UHD_DIR})

if(NOT UHD_FOUND)
    message(STATUS "UHD not found; Building UHD")
    set(UHD_DIR ${_UHD_DIR})

    include(ExternalProject)
    ExternalProject_Add(
            uhd
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/uhd/host
            BINARY_DIR ${CMAKE_BINARY_DIR}/uhd-build
            CONFIGURE_COMMAND
                ${CMAKE_COMMAND}
                -DCMAKE_INSTALL_PREFIX=${UHD_DIR}
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
            OUTPUT ${_UHD_STATIC_LIB}
            COMMAND ${CMAKE_COMMAND} -E echo "Waiting for libuhd.a to finish building"
            DEPENDS uhd
    )

    set(UHD_INCLUDE_DIRS ${UHD_DIR}/include)
    set(UHD_STATIC_LIB_LINK_FLAG "-Wl,-whole-archive ${_UHD_STATIC_LIB} -Wl,-no-whole-archive")
    set(UHD_STATIC_LIB_DEPS "dl;pthread;boost_chrono;boost_date_time;boost_filesystem;boost_program_options;boost_serialization;boost_thread")

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
else()
    message(STATUS "UHD found")
    include(UHDMinDepVersions)
    set(UHD_BOOST_REQUIRED_COMPONENTS
            chrono
            date_time
            filesystem
            program_options
            serialization
            system
            thread
    )
    include(UHDBoost)
    add_custom_target(uhd DEPENDS ${_UHD_LIB_PATH})
endif()

add_custom_target(uhd_static_lib DEPENDS ${_UHD_LIB_PATH})
link_directories(${Boost_LIBRARY_DIRS})
