# 51Degrees Ip Intelligence API

![51Degrees](https://51degrees.com/img/logo.png?utm_source=github&utm_medium=repository&utm_campaign=c_open_source&utm_content=readme_main "Data rewards the curious") **IP Intelligence in C**

[C Documentation](https://51degrees.com/ip-intelligence-cxx/4.1/modules.html) and the [C++ Documentation](https://51degrees.com/ip-intelligence-cxx/4.1/namespaces.html).

The 51Degrees IP intelligence API is built on the 51Degrees [common API](https://github.com/51Degrees/common-cxx).

## Pre-requisites

### Data file

In order to perform IP intelligence, you will need to obtain a 51Degrees data file.  Lite data file can be obtained for free as specified in the [`ip-intelligence-data/README`](https://github.com/51Degrees/ip-intelligence-data/). To obtain an Enterprise data file please [contact us](https://51degrees.com/contact-us).

By default, the downloaded data file has to be placed in the 'ip-intelligence-data' sub-folder of this repo for the tests and examples to work.

### Fetching sub-modules

This repository has sub-modules that must be fetched.
If cloning for the first time, use:

```sh
git clone --recurse-submodules https://github.com/51Degrees/ip-intelligence-cxx.git
```

If you have already cloned the repository and want to fetch the sub modules, use:

```sh
git submodule update --init --recursive
```

If you have downloaded this repository as a zip file then these sub modules need to be downloaded separately as GitHub does not include them in the archive.

## Build tools

### Windows

You will need either Visual Studio 2017/2019 or the [C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) installed.
If you have Visual Studio Code, you'll still need to install the build tools from the link above.

### Linux

You will need [CMake 3.10](https://cmake.org/) or greater installed to build the project. In addition, you will need a C++ compiler which supports C++11. The compiler will and other build tools will be selected by CMake automatically based on your environment.

## Installing

### Using CMake

To build the make files required to build, open a `bash` or `Visual Studio Developer Command Prompt` terminal and run

```sh
mkdir build
cd build
cmake .. 
```

Note: on an x64 Windows system, it is necessary to add `-A x64` as CMake will build a Win32 Solution by default.

Then build the whole solution with

```sh
cmake --build . --config Release
```

Libraries are output to the `lib/` directory, and executables like examples and tests are output to the `bin/` directory.

### Using Visual Studio

Calling `CMake` in an MSVC environment (as described in the [Using CMake](#using-cmake) section) will produce a Visual Studio solution with projects for all libraries, examples, and tests. However, it is preferable to use the dedicated Visual Studio solution in the `VisualStudio/` directory. If the dedicated Visual Studio solution is used, re-targeting the solution might be required for VS2019 IDE.

### Build Options

For build options, see [Common API](https://github.com/51Degrees/common-cxx/blob/master/readme.md)

## Tests

All unit, integration, and performance tests are built using the [Google test framework](https://github.com/google/googletest).

### Testing with CMake

CMake automatically pulls in the latest Google Test from GitHub.

Building the project builds multiple test executables to the `bin/` directory: `IpiTests` and any common testing included in the common-cxx library.

These can be run by calling

```sh
ctest
```

If CMake has been used in an MSVC environment, then the tests will be set up and discoverable in the Visual Studio solution `51DegreesIpIntelligence` created by CMake.

### Testing with Visual Studio

Tests in the Visual Studio solution automatically install the GTest dependency via a NuGet package. However, in order for the tests to show up in the Visual Studio test explorer, the [Test Adapter for Google Test](https://marketplace.visualstudio.com/items?itemName=VisualCPPTeam.TestAdapterforGoogleTest) extension must be installed.

The VisualStudio solution includes `FiftyOne.IpIntelligence.Tests`, which can be run through the standard Visual Studio test runner.

## Referencing the API

### Adding references with CMake

When building using CMake, static libraries are built in stages. These can be included in an executable just as they are in the examples. If these are included through a CMake file, the dependencies will be resolved automatically. However, if linking another way, all dependencies will need to be included from the `lib/` directory. For example, to use the IP Intelligence C API, the following static libraries would need to be included:

- `fiftyone-common-c`
- `fiftyone-ip-intelligence-c`

and for the IP Intelligence C++ API, the following are needed in addition:

- `fiftyone-common-cxx`
- `fiftyone-ip-intelligence-cxx`

The CMake project also builds a single shared library containing everything. This is named `fiftyone-ip-intelligence-complete` and can be referenced by any C or C++ project.

### Adding references with Visual Studio

The Visual Studio solution contains static libraries which have all the dependencies set up correctly, so referencing these in a Visual Studio solution should be fairly self explanatory.

## Examples

There are several examples available, the source files can be found in the `examples/` folder. The examples are written in C or CPP.

### ⚠️ Required files

See `ip-intelligence-data/README` ([local](./ip-intelligence-data/README) / [GitHub](https://github.com/51Degrees/ip-intelligence-data/)) on how to pull and/or generate necessary files.

### Running examples in Visual Studio

All the examples are available to run in the `VisualStudio/IpIntelligence.sln` solution.

### IP Intelligence Examples

|Example|Description|Language|
|-------|-----------|--------|
|Getting Started|This example shows how to get set up with an IP Intelligence engine and begin using it to process IP addresses.|C / C++|
|Meta Data|This example shows how to interrogate the meta data associated with the contents of an IP Intelligence data file.|C++|
|MemIpi|This example measures the memory usage of the IP Intelligence process.|C|
|Offline Processing|This example shows how process data for later viewing using an IP Intelligence data file.|C|
|PerfIpi|Command line performance evaluation program which takes a file of IP addresses and returns a performance score measured in detections per second per CPU core.|C|
|ProcIpi|Command line process which takes an IP address via stdin and return IP intelligence properties via stdout.|C|
|Reload From File|This example illustrates how to reload the data file from the data file on disk without restarting the application.|C / C++|
|Reload From Memory|This example illustrates how to reload the data file from a continuous memory space that the data file was read into without restarting the application.|C / C++|
|Strongly Typed|This example  takes some IP addresses and returns the value of the AverageLocation property as a coordinate which is a pair of float.|C / C++|
