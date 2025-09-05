cmake_minimum_required(VERSION 3.24)
set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Minimum required versions for MSVC
set (MIN_VS_TOOLSET_VERSION 142)
set (MIN_VS_PLATFORM_VERSION 10.0.18362.0)

if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
    message(FATAL_ERROR "Require at least gcc-5.1")
endif()

include(CMakeDependentOption)

project(51DegreesCommon VERSION 4.0.1 LANGUAGES CXX C)

if (MSVC)
# Check minimum required versions for MSVC
    if (CMAKE_VS_PLATFORM_TOOLSET MATCHES "v([0-9]+)")
        if (${CMAKE_MATCH_1} LESS MIN_VS_TOOLSET_VERSION)
	        message(FATAL_ERROR "Require at least Platform Toolset v${MIN_VS_TOOLSET_VERSION}. These can be installed by Visual Studio Installer or as part of C++ Build Tools which can be downloaded from https://visualstudio.microsoft.com/visual-cpp-build-tools/")
        endif()
    else()
        message(FATAL_ERROR "Platform toolset version should start with 'v' and followed by numbers. Got ${CMAKE_VS_PLATFORM_TOOLSET}.")
    endif()
	
	if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION VERSION_LESS ${MIN_VS_PLATFORM_VERSION})
        message(FATAL_ERROR "Require at least windows SDK version ${MIN_VS_PLATFORM_VERSION} which can be downloaded from https://developer.microsoft.com/en-us/windows/downloads/sdk-archive/")
    endif()
endif()

include(CTest)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(Threads REQUIRED)

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
	set(IS_ARM TRUE)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT MSVC)
	message("-- Setting default CMAKE_BUILD_TYPE  to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

#Compute is test coverage enabled
string(TOLOWER "${CMAKE_BUILD_TYPE}" lower_CMAKE_BUILD_TYPE)
if (NOT MSVC AND "${lower_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	set(TESTCOVERAGE_ENABLED TRUE)
else()
	set(TESTCOVERAGE_ENABLED FALSE)
endif()
message("-- TESTCOVERAGE_ENABLED=${TESTCOVERAGE_ENABLED}")

if (NOT MSVC)
	if (lower_CMAKE_BUILD_TYPE MATCHES "^debug")
		set(COMPILE_OPTION_DEBUG -D_DEBUG)
	endif()
endif()

option(32bit "32bit" OFF)
option(MemoryOnly "MemoryOnly" OFF)
option(NoThreading "NoThreading" OFF)
option(ExceptionsDisabled "ExceptionsDisabled" OFF)
option(LargeDataFileSupport "LargeDataFileSupport" OFF)

if (32bit AND NOT IS_ARM)
	message("-- 32 bit compilation")
	if (NOT MSVC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
	else()
		set(CMAKE_VS_PLATFORM_NAME "Win32")
	endif()
endif()

if (NOT MSVC AND NOT APPLE)
    find_package(GccAtomic)
    set(EXE_LINKER_FLAGS "${EXE_LINKER_FLAGS} -latomic")
endif()

if (MemoryOnly)
	message("-- Memory only compilation (FIFTYONE_DEGREES_MEMORY_ONLY) is enabled")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFIFTYONE_DEGREES_MEMORY_ONLY")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFIFTYONE_DEGREES_MEMORY_ONLY")
endif()

if (NoThreading)
	message("-- No Threading compilation (FIFTYONE_DEGREES_NO_THREADING) is enabled")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFIFTYONE_DEGREES_NO_THREADING")
endif()

if (ExceptionsDisabled)
	message("-- Exceptions disable compilation (FIFTYONE_DEGREES_EXCEPTIONS_DISABLED) is enabled")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFIFTYONE_DEGREES_EXCEPTIONS_DISABLED")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFIFTYONE_DEGREES_EXCEPTIONS_DISABLED")
endif()

if (MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _UNICODE /D UNICODE")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT MSVC)
	include_directories(/usr/local/include)
	link_directories(/usr/local/lib)
endif()

# Common

FILE(GLOB COMC_SRC ${CMAKE_CURRENT_LIST_DIR}/*.c)
FILE(GLOB COMC_H ${CMAKE_CURRENT_LIST_DIR}/*.h)
add_library(fiftyone-common-c ${COMC_SRC} ${COMC_H})
target_link_libraries(fiftyone-common-c	${CMAKE_THREAD_LIBS_INIT} ${GCCLIBATOMIC_LIBRARY})

FILE(GLOB COMCPP_SRC ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
FILE(GLOB COMCPP_H ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
add_library(fiftyone-common-cxx ${COMCPP_SRC} ${COMCPP_H})
target_link_libraries(fiftyone-common-cxx fiftyone-common-c)

set_target_properties(fiftyone-common-c fiftyone-common-cxx	PROPERTIES FOLDER "Common")
if (LargeDataFileSupport)
	# global -- to propagate through whole build graph, including tests
	add_compile_definitions(FIFTYONE_DEGREES_LARGE_DATA_FILE_SUPPORT)
	# global -- in case sibling lib uses stdio as well.
	add_compile_definitions(_FILE_OFFSET_BITS=64)
endif ()
if (MSVC)
	target_compile_options(fiftyone-common-c PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
	target_compile_options(fiftyone-common-cxx PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX" "/EHsc")
else ()
	target_compile_options(fiftyone-common-c PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror" "-Wno-atomic-alignment")
	target_compile_options(fiftyone-common-cxx PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
	if (TESTCOVERAGE_ENABLED)
		message("-- fiftyone-common-c(xx)-cov adding targets with code coverage")
		add_library(fiftyone-common-c-cov ${COMC_SRC} ${COMC_H})
		target_link_libraries(fiftyone-common-c-cov	${CMAKE_THREAD_LIBS_INIT} ${GCCLIBATOMIC_LIBRARY})
		add_library(fiftyone-common-cxx-cov ${COMCPP_SRC} ${COMCPP_H})
		target_link_libraries(fiftyone-common-cxx-cov fiftyone-common-c-cov)
		target_compile_options(fiftyone-common-c-cov PRIVATE "--coverage")
		target_compile_options(fiftyone-common-cxx-cov PRIVATE "--coverage")
		set_target_properties(fiftyone-common-c-cov fiftyone-common-cxx-cov	PROPERTIES FOLDER "Common") 
		target_compile_options(fiftyone-common-c-cov PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror" "-Wno-atomic-alignment")
		target_compile_options(fiftyone-common-cxx-cov PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
	endif()
endif()

# Tests
# BUILD_TESTING is true by default, but in some cases it is desirable to turn off

cmake_dependent_option(FIFTYONE_COMMON_CXX_BUILD_TESTING "" ON "BUILD_TESTING" OFF)
message("-- FIFTYONE_COMMON_CXX_BUILD_TESTING=${FIFTYONE_COMMON_CXX_BUILD_TESTING}")

if(BUILD_TESTING)
	# Examples
	add_executable(CachePerf ${CMAKE_CURRENT_LIST_DIR}/performance/CachePerf.c)
	target_link_libraries(CachePerf	fiftyone-common-c)
	if (MSVC)
		target_compile_options(CachePerf PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
		target_link_options(CachePerf PRIVATE "/WX")
	else ()
		target_compile_options(CachePerf PRIVATE ${COMPILE_OPTION_DEBUG} "-Werror")
	endif()
	set_target_properties(CachePerf	PROPERTIES FOLDER "Examples/Common") 

	# Download and unpack googletest at configure time
	configure_file(${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt.in googletest-download/CMakeLists.txt)
	execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
	if(result)
		message(FATAL_ERROR "-- CMake step for googletest failed: ${result}")
	endif()
	execute_process(COMMAND ${CMAKE_COMMAND} --build .
		RESULT_VARIABLE result
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
	if(result)
		message(FATAL_ERROR "-- Build step for googletest failed: ${result}")
	endif()

	# Prevent overriding the parent project's compiler/linker
	# settings on Windows
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

	# Add googletest directly to our build. This defines
	# the gtest and gtest_main targets.
	add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
		${CMAKE_CURRENT_BINARY_DIR}/googletest-build
		EXCLUDE_FROM_ALL)

	# The gtest/gtest_main targets carry header search path
	# dependencies automatically when using CMake 2.8.11 or
	# later. Otherwise we have to add them here ourselves.
	if (CMAKE_VERSION VERSION_LESS 2.8.11)
		include_directories("${gtest_SOURCE_DIR}/include")
	endif()

	include(GoogleTest)
endif()

if(FIFTYONE_COMMON_CXX_BUILD_TESTING)
	set(COM_TEST ${CMAKE_CURRENT_LIST_DIR}/tests)
	FILE(GLOB COM_TEST_SRC ${COM_TEST}/*.cpp)
	FILE(GLOB COM_TEST_H ${COM_TEST}/*.hpp)
	add_executable(CommonTests ${COM_TEST_SRC} ${COM_TEST_H})
	if (MSVC)
		target_compile_options(CommonTests PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
		target_link_options(CommonTests PRIVATE "/WX")
	else ()
		target_compile_options(CommonTests PRIVATE ${COMPILE_OPTION_DEBUG})
		if (NOT MINGW)
			target_link_options(CommonTests PRIVATE "-fsanitize=address")
		endif()
		if (lower_CMAKE_BUILD_TYPE STREQUAL "debug")
			message("-- CommonTests Linker options for code coverage")
			target_link_options(CommonTests PRIVATE "-fprofile-arcs" "-ftest-coverage")
		endif()

	endif()

	if (TESTCOVERAGE_ENABLED)
		message("-- CommonTests linking fiftyone-common-cxx-cov lib with coverage")
		target_link_libraries(CommonTests fiftyone-common-cxx-cov gtest_main)
	else()
		target_link_libraries(CommonTests fiftyone-common-cxx gtest_main)
	endif()

	if (LargeDataFileSupport)
		# global -- to propagate through whole build graph, including tests
		add_compile_definitions(FIFTYONE_DEGREES_LARGE_DATA_FILE_SUPPORT)
		# global -- in case sibling lib uses stdio as well.
		add_compile_definitions(_FILE_OFFSET_BITS=64)
	endif ()

	gtest_discover_tests(CommonTests PROPERTIES TEST_DISCOVERY_TIMEOUT 600 DISCOVERY_MODE PRE_TEST)
	set_target_properties(CommonTests PROPERTIES FOLDER "Tests") 

	if (CMAKE_COMPILER_IS_GNUCC)
		target_compile_options(CommonTests PRIVATE "-Wall" "-Werror" "-Wno-unused-variable" "-Wno-unused-result" "-Wno-unused-but-set-variable")
	endif()
endif()
