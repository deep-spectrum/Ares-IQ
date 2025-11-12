# Contribution tips

Before contributing an extension, please read this page as it may save some headache down the road.

## 1. Contributing Shared Libraries

Some spectrum analyzer APIs are proprietary and are only distributed as shared libraries. This section
describes how to contribute a shared library in an extension.

### 1.1 Check the license

Before contributing a shared library, the  license should be read carefully first. Something to look for in the 
license is whether the library can be redistributed. If the library cannot be redistributed, then an extension 
cannot be written for it. 

### 1.2 Shared library dependencies

Sometimes, a shared library requires other shared libraries to be loaded at runtime. If there is not one
in the default system path, then it is necessary to add it to the extension as well. An example of this
is the Signal Hound BB API (requires the ftd2xx.so shared library). To deal with this, there are two
paths forward:

1. Load the shared library before loading the one you are creating
2. Modify the shared library to search for shared libraries within its own directory (if it doesn't already)

Both options will be discussed below:

#### 1.2.1 Loading the dependencies first

1. Set up cmake so the shared library dependencies are installed in the same location as the shared library
2. In the \_\_init\_\_.py file, import cdll from ctypes and use the `LoadLibrary` method to load the shared library 
   before importing the built shared library.

Below is an example of how to load shared library dependencies using the Signal Hound BB60 API.

```py title="__init__.py"
from __future__ import annotations
from ctypes import cdll
from pathlib import Path

_site_package = Path(__file__).parent.resolve() / "lib"
cdll.LoadLibrary(f"{_site_package}/libftd2xx.so")

from ._sh_bb60 import ...

__all__ = [ ... ]
```

#### 1.2.2 Modifying the library run path

???+ warning "Check the license"

     Check the license to see if the shared library can be modified. If not, then you are stuck with loading the
     dependencies first.

If you prefer a cleaner \_\_init\_\_.py, then modifying the search path for the shared library is necessary. This is
relatively simple to do with [patchelf](https://manpages.ubuntu.com/manpages/jammy/man1/patchelf.1.html). Below is an
example of modifying a shared library to look for other shared libraries in the same directory it is in.

1. Run the `ldd` command on the shared library. This should tell you if there are any missing dependencies. If there are
   no missing dependencies, then there is no need to modify the shared library. Below is an example output:
   ```none title="Example output from ldd command"
   $ ldd libfoo.so
	   linux-vdso.so.1 (0x00007ffc457b2000)
	   libusb-1.0.so.0 => /lib/x86_64-linux-gnu/libusb-1.0.so.0 (0x000070c71aca1000)
	   libftd2xx.so => not found
	   libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x000070c71ac9c000)
	   libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x000070c719800000)
	   libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x000070c71abb5000)
	   libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x000070c71ab93000)
	   libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x000070c719400000)
	   libudev.so.1 => /lib/x86_64-linux-gnu/libudev.so.1 (0x000070c719bd6000)
	   /lib64/ld-linux-x86-64.so.2 (0x000070c71acef000)
   ```
2. If there is a library missing, it is worth checking if the shared library is looking for its dependencies in
   a certain location relative to the shared library file. This can be done with the [readelf](https://man7.org/linux/man-pages/man1/readelf.1.html) command.
   If it is, then the shared library should not be modified and the dependencies should be installed in the location
   the shared library is expecting them to be (as long as the run path is relative to the shared library).
   ```none title="Example output from readelf command"
   $ readelf -d libfoo.so | grep RUNPATH
    0x000000000000001d (RUNPATH)            Library runpath: [$ORIGIN/dependencies]
   ```
3. If the grep result comes up empty, then modifying the shared library is necessary. To do this, you can use the patchelf
   command:
   ```bash title="Example usage of patchelf"
   patchelf --set-rpath '$ORIGIN' libfoo.so
   ```
   After running patchelf, the dependency should be able to be resolved as long as it is in the same directory:
   ```none title="ldd command after running patchelf"
   $ ldd libfoo.so
	   linux-vdso.so.1 (0x00007ffc457b2000)
	   libusb-1.0.so.0 => /lib/x86_64-linux-gnu/libusb-1.0.so.0 (0x000070c71aca1000)
	   libftd2xx.so => path/to/libftd2xx.so (0x00007a1be0000000)
	   libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x000070c71ac9c000)
	   libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x000070c719800000)
	   libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x000070c71abb5000)
	   libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x000070c71ab93000)
	   libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x000070c719400000)
	   libudev.so.1 => /lib/x86_64-linux-gnu/libudev.so.1 (0x000070c719bd6000)
	   /lib64/ld-linux-x86-64.so.2 (0x000070c71acef000)
   ```
   
### 1.3 CMakeLists.txt set up

If shared libraries are used, then they should be copied into the package during the build. This will require
a few extra cmake directives to do so. This example will build a Python package that uses the bb series API from
Signal Hound. The first thing that should be set in the CMakeLists.txt are the library dependencies. To make
things more maintainable and easier to read, it is recommended that a variable is set with the library paths:

```cmake
set(SH_BB_LIBS
        ${CMAKE_SOURCE_DIR}/lib/signal-hound/bb_series/libbb_api.so.5
        ${CMAKE_SOURCE_DIR}/lib/signal-hound/bb_series/libftd2xx.so
)
```

You can then add those libraries to your python module target:

```cmake
python_module(_sh_bb60 src/signal-hound/bb_series/bb_device.cpp
        DESTINATION ${PACKAGE_NAME}/signal_hound
        DEPENDENCIES capture_progress
        LIBS capture_progress ${SH_BB_LIBS}
        INSTALL_LIBS ${SH_BB_LIBS}
)
```

This is what the CMakeLists.txt should look like in this example:

```cmake title="CMakeLists.txt"
set(SH_BB_LIBS
        ${CMAKE_SOURCE_DIR}/lib/signal-hound/bb_series/libbb_api.so.5
        ${CMAKE_SOURCE_DIR}/lib/signal-hound/bb_series/libftd2xx.so
)

python_module(_sh_bb60 src/signal-hound/bb_series/bb_device.cpp
        DESTINATION ares_iq_ext/signal_hound
        DEPENDENCIES capture_progress
        LIBS capture_progress ${SH_BB_LIBS}
        INSTALL_LIBS ${SH_BB_LIBS}
)
```

## 2. Contributing In Tree Libraries

Another way to contribute a library is to put it in the file tree. These libraries are libraries that we have the source
code to and can either be written ourselves or can be external projects. If they are external project, they can be added
as submodules or get pulled in through fetch content in cmake. However, if they are written specifically for this
project, it is advised that they are added in tree. This section will go over the different scenarios for pulling in
libraries. Additionally, using static libraries is preferred.

### 2.1 Fetch content

If an external library uses a modern cmake system, then fetch content can be a good option to pull in an external
library. However, the drawback of using this method is the computer must be connected to the internet in order to
build the library. In order to pull in a dependency via fetch content, then fetch content library should be included in 
the CMakeLists.txt file if it is not already.

```cmake title="Example usage of FetchContent"
include(FetchContent)  # Do not include this if it is already present
FetchContent_Declare(
        foo
        GIT_REPOSITORY https://...
        GIT_TAG master
)
FetchContent_MakeAvailable(foo)
...
target_link_libraries(target PRIVATE foo ...)
```

### 2.2 Submodules

Submodules are another way to include an external library. Submodules can be useful for pinning things to a certain 
commit or branch, and they only require an internet connection on the initialization of the repository. Additionally,
if an external library has a poorly set up cmake build system or uses an entirely different build system, then this
option is probably the best choice. If a Library has a poor cmake set up or does not use cmake, then you need to
make a separate cmake file and export a static library. Please refer to 
[FindUHD.cmake](https://github.com/deep-spectrum/Ares-IQ/blob/master/ares-iq-extensions/cmake/FindUHD.cmake)
to see how to deal with a poorly set up cmake system or an entirely different build system.

```cmake title="Library has a nice cmake set up"
add_subdirectory(extern/foo)
...
target_link_libraries(target PRIVATE foo ...)
```

### 2.3 Native library

If there is a need to common behavior across multiple modules, but there is no open source library available, then
it is necessary to write a native library. Native libraries should have their own CMakeLists.txt file that exports
a static library. Below is an example cmake file for a native library.

```cmake  title="Example CMakeLists.txt for in-tree libraries"
cmake_minimum_required(VERSION 3.15)
project(foo)

set(CMAKE_CXX_STANDARD 11)

add_library(foo STATIC ...)
set_target_properties(foo PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(foo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
```

## 3. Code Samples

The following snippets are examples of how the `CMakeLists.txt` and `__init__.py` files should look.

??? tip "Destination"

    The destination should be installed in the same directory as the `__init__.py`. For example, if the `__init__.py`
    for this module is in `src/ares_iq_ext/bar`, then the destination should be `ares_iq_ext/bar`.

```cmake title="Sample usage of python_module() in CMakeLists.txt"
set(PACKAGE_NAME "ares_iq_ext")
python_package(src/${PACKAGE_NAME} DESTINATION ${PACKAGE_NAME})
...
set(FOO_LIBS
    ${CMAKE_SOURCE_DIR}/lib/bar/bar.so
)
python_module(_bar src/foo/bar.cpp ...
    DESTINATION ${PACKAGE_NAME}/bar
    DEPENDENCIES capture_progress ...
    LIBS capture_progress ${FOO_LIBS} ...
    DEFINITIONS FOO_ENABLED=1 ...
    INSTALL_LIBS ${FOO_LIBS} ...
    PYBIND_LIBS pybind11::headers pybind11::embed ...
)
```

??? note "\_\_init\_\_.py placement"

    The \_\_init\_\_.py file is located in the `src/ares_iq_ext/bar` directory in this example. This is because the
    `python_package()` cmake function preserves the structure of the python package being installed, including 
    subpackages.


```python title="Sample __init__.py for the _bar module"
from __future__ import annotations

from ._bar import __doc__, ...

__all__ = [__doc__, ...]
```
