if(APPLE)
    set(origin_token "@loader_path")
else()
    set(origin_token "$ORIGIN")
endif()

#[[
  python_module(<name> <src_dir>
      [DESTINATION <path>]
      [DEPENDENCIES <target>...]
      [LIBS <lib>...]
      [DEFINITIONS <def>...]
      [INSTALL_LIBS <file>...])
  Defines and installs a Python C++ extension module.

  Arguments:
    <name>         – Target name for the module.
    <src_dir>      – Path to the .cpp file for pybind11.
    DESTINATION    – Install path (relative to prefix).
    DEPENDENCIES   – Other targets this depends on.
    LIBS           – Libraries to link against.
    DEFINITIONS    – Preprocessor definitions.
    INSTALL_LIBS   – Shared libraries to install with the module.
]]
function(python_module name src_dir)
    # Required arguments
    set(options)
    set(one_value_args DESTINATION)
    set(multi_value_args DEPENDENCIES LIBS DEFINITIONS INSTALL_LIBS)
    cmake_parse_arguments(MOD "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    # Create Python module
    python_add_library(${name} MODULE ${src_dir} WITH_SOABI)

    # Add dependencies if provided
    if(MOD_DEPENDENCIES)
        add_dependencies(${name} ${MOD_DEPENDENCIES})
    endif()

    # Link libraries
    target_link_libraries(${name} PRIVATE pybind11::headers ${MOD_LIBS})

    # Add compile definitions
    if(MOD_DEFINITIONS)
        target_compile_definitions(${name} PRIVATE ${MOD_DEFINITIONS})
    endif()

    # Install target
    install(TARGETS ${name} DESTINATION ${MOD_DESTINATION})

    # Optional library installs (e.g., shared libs)
    if(MOD_INSTALL_LIBS)
        install(FILES ${MOD_INSTALL_LIBS} DESTINATION ${MOD_DESTINATION}/lib)
        if(APPLE)
            set(origin_token "@loader_path")
        else()
            set(origin_token "$ORIGIN")
        endif()
        set_property(TARGET ${name} PROPERTY INSTALL_RPATH "${origin_token}/lib")
    endif()
endfunction()

#[[
  python_package(<package_src_dir>)
  Installs a Python package given the relative path to the package.
]]
function(python_package package_src_dir)
    get_filename_component(abs_src_dir ${package_src_dir} ABSOLUTE)
    if(NOT IS_DIRECTORY ${abs_src_dir})
        message(FATAL_ERROR "install_python_package: '${package_src_dir}' is not a valid directory")
    endif()

    file(RELATIVE_PATH rel_install_path ${CMAKE_SOURCE_DIR}/src ${abs_src_dir})

    install(
            DIRECTORY ${abs_src_dir}/
            DESTINATION ${rel_install_path}
            FILES_MATCHING
            PATTERN "*.py"
    )
endfunction()
