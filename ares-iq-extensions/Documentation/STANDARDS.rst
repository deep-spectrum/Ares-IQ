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
about code formatting as there is a tool that will do that for you automatically. However,
there are a few things that we ask for you to keep in mind that the formatting tool will not
catch.

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

    // Not OK
    switch(foo) {
    case 0: doSomething();
        break;
    case 1: doOtherThing();
        break;
    default: doError();
        break;
    }

    // OK
    switch(foo) {
    case 0: {
        doSomething();
        break;
    }
    case 1: {
        doOtherThing();
        break;
    }
    default: {
        doError();
        break;
    }
    }

3.2) Constants
--------------
In C++, there are a few ways to define constants, however, not all ways are equal. Since this
application is running on a processor with plenty of resources at its disposal, use of macros
as constants is discouraged and should be used sparingly.

.. code-block:: C++

    // Not OK
    #define FOO_X 27

    // OK
    constexpr uint32_t foo_x = 27;

    // OK
    const uint32_t foo_x = 27;

3.3) Control Flow
-----------------
Control flow is an integral part of programs, however, if used improperly, it will make the
code look like a giant heap of dog shit. There are a few common practices to consider when
using control flow.

3.3.1) Nesting
^^^^^^^^^^^^^^
Nesting control flow is OK for some things, however, exesive nesting becomes a problem. Only 1
level of nesting is allowed. If you need 2 or more levels of nesting, maybe consider
converting your flow chart into a state machine or breaking things up into multiple functions.
Additionally, C and C++ implement `short-circuit evaluation`_, so that should be used to reduce
nesting.

.. _short-circuit evaluation: https://www.geeksforgeeks.org/c/short-circuit-evaluation-in-programming/

3.3.2) Cyclomatic Redundancy
^^^^^^^^^^^^^^^^^^^^^^^^^^^^
There is a metric in software that measures the complexity called "Strict Cyclomatic Complexity."
This not only measures the amount of paths (via nesting) your software can take, it also measures
considers the amount of conditions that need to be taken into consideration in order to take that
path. Consider the two function implementations below:

.. code-block:: C

    void fun1(bool a, bool b) {
        if (a) {
            if (b) {
                ...
            }
        }
    }

    void fun2(bool a, bool b) {
        if (a && b) {
            ...
        }
    }

Say that ``fun1`` and ``fun2`` do the exact same thing. Under normal McCabe Cyclomatic Complexity,
``fun2`` (MCC of 2) would be considered better because it has 1 less branch than ``fun1`` (MCC of 3).
However, under Strict Cyclomatic Complexity, they would be equivalent because each logical operation
adds a branch.

To maintain simplicity and maintainability of the code, it is asked that most functions have an SCC
of 10 or below and no functions exceed an SCC of 15.

3.3.3) Centralized exiting of functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Albeit deprecated by some people, the equivalent of the goto statement is used frequently by
compilers in form of the unconditional jump instruction.

The goto statement comes in handy when a function exits from multiple locations and some
common work such as cleanup has to be done. If there is no cleanup needed then just return
directly.

Choose label names which say what the goto does or why the goto exists. An example of a good
name could be ``out_free_buffer:`` if the goto frees ``buffer``. Avoid using GW-BASIC names
like ``err1:`` and ``err2:``, as you would have to renumber them if you ever add or remove
exit paths, and they make correctness difficult to verify anyway.

The rationale for using gotos is:

* unconditional statements are easier to understand and follow
* nesting is reduced
* errors by not updating individual exit points when making modifications are prevented
* saves the compiler work to optimize redundant code away ;)

.. code-block:: C

    int fun(int a) {
        int result = 0;
        char *buffer;

        buffer = malloc(SIZE);
        if (!buffer) {
            return -ENOMEM;
        }

        ...

        if (condition1) {
            while (loop1) {
                ...
            }
            result = 1;
            goto out_free_buffer;
        }
        ...
    out_free_buffer:
        free(buffer);
        return result;
    }

A common type of bug to be aware of is ``one err bugs`` which look like this:

.. code-block:: C

    err:
        free(foo->bar);
        free(foo);
        return ret;

The bug in this code is that on some exit paths foo is NULL. Normally the fix for this is
to split it up into two error labels ``err_free_bar:`` and ``err_free_foo:``:

.. code-block:: C

    err_free_bar:
        free(foo->bar);
    err_free_foo:
        free(foo);
        return ret;

Ideally you should simulate errors to test all exit paths.

3.4) Use of auto
----------------
The ``auto`` keyword can be helpful in a few situation, however, if used excessively, the
code becomes very difficult to understand and it becomes a pissing game of figuring out what
variable is what type. For example, this is perfectly valid C++ code:

.. code-block:: C++

    auto foo = 27;
    auto bar = "27";

Can you predict what the types of ``foo`` and ``bar`` will be at compile time without using
your IDE? Probably not! That is why it is recommended to only use ``auto`` in certain situations,
like casting to another type or when you need to use a loop iterator. C++11 does not allow ``auto`` in lambda parameters.
Below are some examples of bad uses and good uses:

.. code-block:: C++

    // Not OK
    auto foo = 27;

    // OK
    auto foo = std::chrono::duration<double>(std::chrono::system_clock::now() - _start).count();

    // OK
    for (auto &i : foo) {
        ...
    }

3.5) Commenting
---------------
Comments are good, but there is a danger to over commenting. Do not explain how your code works
in a comment. It is much better to write the code so that the **working** is obvious, and it's
a waste of time to explain badly written code.

Generally, you want your comments to tell WHAT your code does, not HOW. Also, try to avoid
putting comments inside a function body: if the function is so complex that you need to
separately comment parts of it, you should probably take a look at functions. You can make
small comments to note or warn about something particularly clever (or ugly), but try to
avoid excess. Instead, put the comments at the head of the function, telling people what
it does, and possibly WHY it does it.

When documenting an API, it is only necessary to document the header files. The public API
should be documented as well as the protected API (if using polymorphism). There is no
need to document the private API.

When documenting functions, the input parameters as well as the return value should be
documented. Each public and protected member variable should be documented. Each data
structure should be documented. Public facing comments should be styled after
`Doxygen Comment Blocks`_, and private comments should be kept as normal comments (so
they don't get included in the doc generation).

.. _Doxygen Comment Blocks: https://www.doxygen.nl/manual/docblocks.html#specialblock

3.6) Templates
--------------
Something that C++ brings to the table is metaprogramming via templates. These may seem
nice, but it is advised that they are only used for general library implementations where
they are not directly used as bindings.

3.7) Polymorphism
-----------------
Another thing that C++ brings to the table is polymorphism. This allows someone to base
their implementation from an existing class definition. This may seem useful, however,
it can lead to some really bad design decisions that have major impacts on the
performance or understandablity of the code. Polymorphism should not be used for most
cases in this library.

4) Submitting a PR
==================
You wrote an extension and now you want it to be part of the next release. Awesome! There
are a few things we ask of you before submitting a PR.

4.1) Check the SCC of your code
-------------------------------
Run the following command:

.. code-block:: bash

    ./check scc

It should spit out a report of the C++ code base. Fix anything that reports more than
15, and consider simplifying the functions that report between 11 and 15.

4.2) Check the code style
-------------------------
Run the following command:

.. code-block:: bash

    ./check style

Consider fixing all the warnings that get spit out, however, any checks that got disabled
the files and the line numbers need to be noted in the PR.

4.3) Format the code
--------------------
Consistent formatting is necessary to maintain readability of the code base. Luckily, there's
a tool that does it for you already. Just run the following command:

.. code-block:: bash

    ./check format

Just make sure to commit your code after running that command.

4.4) Ready to submit the PR now?
--------------------------------
After running the checks and the formatter, you should be ready to submit your PR now.
We will ask you to generate a PR summary with github copilot, indicate what platform the
bindings are written for, and indicate any clang-tidy checks that got disabled as per
section 4.2.
