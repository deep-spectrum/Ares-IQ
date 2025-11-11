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
2. In the \_\_init\_\_.py file, import cdll from ctypes and use the `LoadLibrary` method to load the shared library before importing the built shared library.

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

!!! warning "Check the license"

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

You then add the target to the CMakeLists.txt with the following directive like normal:

```cmake
python_add_library(_sh_bb60 MODULE src/signal-hound/bb_device.cpp WITH_SOABI)
```

Then, once you declare your target, you need to configure it and link the necessary libraries. Notice that
the shared libraries specified above are being linked to. Additionally, notice that a property is being set
for the shared library being created. This is necessary because it is more convenient for the user to install
the package without needing to install the distributed shared libraries globally.

```cmake
add_dependencies(_sh_bb60 capture_progress)
target_link_libraries(_sh_bb60 PRIVATE pybind11::headers capture_progress ${SH_BB_LIBS})
set_property(TARGET _sh_bb60 PROPERTY
		INSTALL_RPATH "${origin_token}/lib"  # Note that "origin_token" is set earlier in the CMakeLists file.
)
```

After configuring the target, you then want to tell cmake where to install the package and its dependencies. Since the
shared libraries are dependencies of the module being built, then it is necessary to tell cmake to install the shared
libraries along with the module. Since the `INSTALL_RPATH` was set to `"${origin_token}/lib"`, then the libraries
should be installed in the lib subdirectory in the package.

```cmake
install(TARGETS _sh_bb60 DESTINATION ares_iq_ext/signal_hound/bb_series)
install(FILES src/ares_iq_ext/signal-hound/bb_series/__init__.py DESTINATION ares_iq_ext/signal_hound/bb_series)
install(FILES ${SH_BB_LIBS} DESTINATION ares_iq_ext/signal_hound/bb_series/lib)
```

This is what the CMakeLists.txt should look like in this example:

```cmake title="CMakeLists.txt"
set(SH_BB_LIBS
        ${CMAKE_SOURCE_DIR}/lib/signal-hound/bb_series/libbb_api.so.5
        ${CMAKE_SOURCE_DIR}/lib/signal-hound/bb_series/libftd2xx.so
)

python_add_library(_sh_bb60 MODULE src/signal-hound/bb_device.cpp WITH_SOABI)
add_dependencies(_sh_bb60 capture_progress)
target_link_libraries(_sh_bb60 PRIVATE pybind11::headers capture_progress ${SH_BB_LIBS})
set_property(TARGET _sh_bb60 PROPERTY
		INSTALL_RPATH "${origin_token}/lib"  # Note that "origin_token" is set earlier in the CMakeLists file.
)
install(TARGETS _sh_bb60 DESTINATION ares_iq_ext/signal_hound/bb_series)
install(FILES src/ares_iq_ext/signal-hound/bb_series/__init__.py DESTINATION ares_iq_ext/signal_hound/bb_series)
install(FILES ${SH_BB_LIBS} DESTINATION ares_iq_ext/signal_hound/bb_series/lib)
```
