================
Coding Standards
================

This is a short document describing the coding standards of the Ares-IQ Python extensions.

0) When is it Appropriate to Write an Extension
===============================================

Writing Python extensions should be avoided at all costs. If there is a pre-existing Python
library or Python API, then that should be used instead. However, there are 2 situations
where writing a Python extension is appropriate:

1. If there is no Python library or API available from the SDR manufacturer and they only provide a C/C++ API.

2. The existing Python library/API has major performance issues.

The reasoning of adding the extension must be docummented with benchmarks if applicable.

1) Language
===========

The language and standard used is C++11. As painful as it is, this is what pybind11 requires.
If an API uses C, then it should be pretty simple to get it into C++. Just make sure to wrap
the header with the following if it's not already present in the header:

.. code:: C

    extern "C" {}


2) Organization
===============

Project organization is very important for quickly navigating the project so it is paramount
that things are placed in the correct locations.

2.1) Python Bindings
--------------------

To add a Python extension for a specific SDR platform, create a set of new directories
named after the target platform's base name. The directory structure should follow the
format outlined below:

.. code-block:: none

    ares-iq-extensions
    ├── include
    │   └── ares-iq
    │       └── <Platform Name>
    ├── lib
    │   └── <Platform Name>
    └── src
        ├── ares_iq_ext
        │   ├── __init__.py
        │   └── <Platform Name>
        │       └── __init__.py
        └── <Platform Name>

2.1.1) Directory Guidelines
^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. **Header Files**:
   All header files for the platform must be placed in the
   ``include/ares-iq/<Platform Name>`` directory.

2. **Shared Object Files**:
   Place all shared object (``.so``) files in the ``lib/<Platform Name>`` directory.

3. **C++ Source Files**:
   All C++ source files should be stored in the ``src/<Platform Name>`` directory.

4. **Python Package**:
   A Python package for the platform must be created under
   ``src/ares_iq_ext/<Platform Name>``, including the necessary ``__init__.py`` file.

2.2) External Dependencies
--------------------------

Whenever possible, external dependencies should be pulled in via `FetchContent`_ in cmake.
However, that may not always be possible with some libraries. Any libraries that cannot
be pulled in via FetchContent must be placed in the ``extern/`` directory as a
`git submodule`_.

.. _`FetchContent`: https://cmake.org/cmake/help/latest/module/FetchContent.html
.. _git submodule: https://git-scm.com/book/en/v2/Git-Tools-Submodules

2.3) In-Tree Libraries
----------------------

To add an in-tree library, a new directory needs to be created. These directories must
follow this basic structure:

.. code-block:: none

    <Library Name>
    ├── CMakeLists.txt
    ├── include
    │   └── <Library Name>
    └── src

2.3.1) Directory Guidelines
^^^^^^^^^^^^^^^^^^^^^^^^^^^

1. **Header Files**:
   All header files must be placed in the ``include/<Library Name>`` directory.

2. **Source Files**:
   All source files must be placed in the ``src`` directory.

3. **Additional Organization**:
   You may create additional directories within ``include/<Library Name>`` or
   ``src`` for better organization, if necessary.

4. **No Shared Objects**:
   In-tree libraries should not include shared object files. Any pull request
   that includes a shared object will be automatically rejected.

5. **Non-compilation Files**:
   Files not needed for compilation (such as documentation) can deviate from the
   basic structure, as this structure is only required for source code

3) Code Style
===============
This project has a coding style. When writing your code, you don't have to worry too much
about styling. However, there are a few things that we ask for you to keep in mind.

3.1) Braces for single-line control statements
----------------------------------------------
In both C and C++, the curly braces around single-line control statements can be ommitted. However,
in this repository, it is considered bad practice to not wrap the single-line control statements
because it can lead to confusion in the codebase and introduce subtle bugs that are difficult to spot.
All control statements — ``if``, ``else``, ``for``, ``while``, ``do-while``, ``switch``, and ``case`` —
must always have curly braces, even if they only contain a single statement.

.. code-block:: C

    // Not OK
    if (x < 0)
        x = 0;

    // OK
    if (x < 0) {
        x = 0;
    }
