# 51Degrees Common Code Library

![51Degrees](https://51degrees.com/DesktopModules/FiftyOne/Distributor/Logo.ashx?utm_source=github&utm_medium=repository&utm_content=home&utm_campaign=c-open-source "Data rewards the curious") **51Degrees Common C Code**

[Reference Documentation](https://51degrees.com/device-detection-cxx/4.4/index.html "Reference documentation")

# Introduction

The 51Degrees Common Code Library groups common methods and structures used in 51Degrees APIs. This includes methods to stream from files, data sets, collections of items, arrays, memory management among other things. Anything which is not specific to a functional API is contained here as generic implementations which can be reused in multiple specific use cases.

# Naming Conventions

All public methods and structures are prefixed by `fiftyoneDegrees` then the name of the area they belong to, e.g. the `fiftyoneDegreesCollection` prefix indicates it exists in `collection.c/h`.

The next part of a method name is the verb which describes what the method does, common terms are:

| Verb | Meaning |
|-|-|
| Init | Initializes a structure which has already been allocated. This will take a pointer to the structure which is to initialized. |
| Create | Allocates and initializes a new structure, then returns the pointer. |
| Size | Returns the number of bytes which will be allocated by the accompanying `create` or `init` method. Implicitly, `create` or `init` methods which have no accompanying `size` method do not allocate any memory. |
| Get | Returns a value or pointer which may need to be released after use. |
| Add | Add an item to a structure, this may or may not allocate memory. |
| Process | Processes data and populates a structure which is passed in. |
| Free | Frees the memory allocated to a structure. |
| Release | Releases a resource back to the resource manager or whatever structure in charge of handing out references to that particular resource. |

e.g. the `fiftyoneDegreesDataSetGet` returns a pointer to a data set which will need to be released with the `fiftyoneDegreesDataSetRelease`
method.

Finally, the optional last part completes the description of the method's use, e.g. `fiftyoneDegreesDataSetInitFromMemory` or `fiftyoneDegreesDataSetInitFromFile`.

# Installing

## Dependencies

When compiling on Linux, there is a dependency on `libatomic`. This
is usually already present as part of the C libraries, and is included
as part of GCC 5 and above. On other systems, can be installed with:

```
sudo apt-get install libatomic1
```

## Using CMake

To build the make files required to build, open a `bash` or `Visual Studio Developer Command Prompt` terminal and run

```
mkdir build
cd build
cmake .. 
```

Note: on an x64 Windows system, it is necessary to add `-A x64` as CMake will build a Win32 Solution by default.

Then build the whole solution with

```
cmake --build . --config Release
```

Libraries are output to the `lib/` directory, and executables like examples and tests are output to the `bin/` directory.

## Using Visual Studio

Calling `CMake` in an MSVC environment (as described in the [Using CMake](#Using-CMake) section) will produce a Visual Studio solution with projects for all libraries, examples, and tests. However, it is preferable to use the dedicated Visual Studio solution in the `VisualStudio/` directory.

Prerequisites:
- Required Visual Studio 2019 or equivalent [C++ build tool](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
  - Minimum Platform Toolset Version `v142`
  - Minimum Windows SDK Version `10.0.18362.0`

## Build Options

### MemoryOnly

In memory only operation compiling without stream capabilities using the `FIFTYONE_DEGREES_MEMORY_ONLY` directive results in performance improvements. By removing the unnecessary jumps to methods which are only used in stream operation, the performance of in memory collections is improved.

The option is enabled using the `FIFTYONE_DEGREES_MEMORY_ONLY` compile flag, which can also be set through CMake using the `-DMemoryOnly=YES` option, or through Visual Studio using the the `Debug-MemoryOnly` or `Release-MemoryOnly` build configurations.

### NoThreading

In single threaded operation compiling without threading using the `FIFTYONE_DEGREES_NO_THREADING` directive results in performance improvements. By removing logic needed to keep operations thread safe, the performance of many operations are improved.

The option is enabled using the `FIFTYONE_DEGREES_NO_THREADING` compile flag, which can also be set through CMake using the `-DNoThreading=YES` option, or through Visual Studio using the `Debug-Single` or `Release-Single` build configurations.

<p style="color:#FF6000">WARNING: This option should only be enabled in single threaded environments, as it means operations will not be thread safe.</P>

### ExceptionsDisabled

Exceptions are handled in the common library using the `fiftyoneDegreesException` structure. This logic can be disabled using the `FIFTYONE_DEGREES_EXCEPTIONS_DISABLED` directive, resulting in performance improvements.

The options is enabled using the `FIFTYONE_DEGREES_EXCEPTIONS_DISABLED` compile flag, which can also be set through CMake using the `-DExceptionsDisabled=YES` option.

<p style="color:#FF6000">WARNING: When exception handling is disabled, using the functionality in this library incorrectly can result in a segmentation fault instead of an exception being set.<p>

# Tests

All unit, integration, and performance tests are built using the [Google test framework](https://github.com/google/googletest).

## CMake

CMake automatically pulls in the latest Google Test from GitHub.

Building the project builds the test executable `CommonTests` to the `bin/` directory. This can be run directly from a terminal.

These can be run by calling
```
ctest
```

CTest module automatically creates a `BUILD_TESTING` option -- see https://cmake.org/cmake/help/latest/module/CTest.html

When testing some dependent code, running `CommonTests` can be skipped by turning OFF the `FIFTYONE_COMMON_CXX_BUILD_TESTING` option.

If CMake has been used in an MSVC environment, then the tests will be set up and discoverable in the Visual Studio solution `51DegreesCommon` created by CMake.

## Visual Studio

Tests in the Visual Studio solution automatically install the GTest dependency via a NuGet package. However, in order for the tests to show up in the Visual Studio test explorer, the [Test Adapter for Google Test](https://marketplace.visualstudio.com/items?itemName=VisualCPPTeam.TestAdapterforGoogleTest) extension must be installed.

The VisualStudio solution includes `FiftyOne.Common.Tests`, which can be run through the standard Visual Studio test runner. 

# Referencing the library

## CMake

When building using CMake, static libraries are built in stages. These can be included in an executable just as they are in the examples. If these are included through a CMake file, the dependencies will be resolved automatically. However, if linking another way, all dependencies will need to be included from the `lib/` directory. For example, to use the Common C library, the `fiftyone-common-c` static library would need to be included, and for C++, `fiftyone-common-cxx` is needed in addition.

## Visual Studio

The Visual Studio solution contains static libraries which have all the dependencies set up correctly, so referencing these in a Visual Studio solution should be fairly self explanatory.

## Code Coverage

When the project files are generated for a non-MSVC compiler (gcc/clang) with `CMAKE_BUILD_TYPE=Debug` f.e. by running: 
```
cmake . -DCMAKE_BUILD_TYPE=Debug
```

the generated targets will contain code coverage instrumentation instructions to be added to the libraries with `-cov` suffix and `CommonTests` target will be linked with them. During build time `*.gcno` and empty `*.gcda` files are generated.  

During the test run  `*.gcda` files will be filled with coverage data.  

Install gcovr tool by running:
```
pip3 install gcovr
```

For best results make sure you have at least version 7.2.

The following command will generate a detailed report for each file: 
```
mkdir -p coverage; gcovr --html-detail -o coverage/coverage.html; open coverage/coverage.html
```

To check the summary run:
```
gcovr -r . --print-summary
```
