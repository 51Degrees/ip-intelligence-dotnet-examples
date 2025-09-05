cmake_minimum_required(VERSION 3.24)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Include the common API

include(CMakeDependentOption)
cmake_dependent_option(FIFTYONE_COMMON_CXX_BUILD_TESTING "" OFF "BUILD_TESTING" OFF)
option(LargeDataFileSupport "LargeDataFileSupport" ON)

include(${CMAKE_CURRENT_LIST_DIR}/src/common-cxx/CMakeLists.txt NO_POLICY_SCOPE)

#Compute is test coverage enabled
string(TOLOWER "${CMAKE_BUILD_TYPE}" lower_CMAKE_BUILD_TYPE)
if (NOT MSVC AND "${lower_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	set(TESTCOVERAGE_ENABLED TRUE)
else()
	set(TESTCOVERAGE_ENABLED FALSE)
endif()
message("-- TESTCOVERAGE_ENABLED=${TESTCOVERAGE_ENABLED}")

project(51DegreesIpIntelligence VERSION 4.0.1 LANGUAGES CXX C)

set(IPI ${CMAKE_CURRENT_LIST_DIR}/src)
set(IPI_GRAPH ${CMAKE_CURRENT_LIST_DIR}/src/ip-graph-cxx)

# IP Intelligence libraries
FILE(GLOB IPIC_SRC ${IPI}/*.c ${IPI_GRAPH}/*.c)
FILE(GLOB IPIC_H ${IPI}/*.h ${IPI_GRAPH}/*.h)
add_library(fiftyone-ip-intelligence-c ${IPIC_SRC} ${IPIC_H})
if (MSVC)
	target_link_libraries(fiftyone-ip-intelligence-c fiftyone-common-c)
	target_compile_options(fiftyone-ip-intelligence-c PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
else()
	target_link_libraries(fiftyone-ip-intelligence-c fiftyone-common-c m)
endif()

FILE(GLOB IPICPP_SRC ${IPI}/*.cpp)
FILE(GLOB IPICPP_H ${IPI}/*.hpp)
add_library(fiftyone-ip-intelligence-cxx ${IPICPP_SRC} ${IPICPP_H})
target_link_libraries(fiftyone-ip-intelligence-cxx
	fiftyone-ip-intelligence-c
	fiftyone-common-cxx)

if (TESTCOVERAGE_ENABLED)
	message("-- fiftyone-ip-intelligence-c(xx)-cov adding targets with code coverage")
	add_library(fiftyone-ip-intelligence-c-cov ${IPIC_SRC} ${IPIC_H})
	if (MSVC)
		target_link_libraries(fiftyone-ip-intelligence-c-cov fiftyone-common-c-cov)
		target_compile_options(fiftyone-ip-intelligence-c-cov PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
	else()
		target_link_libraries(fiftyone-ip-intelligence-c-cov fiftyone-common-c-cov m)
	endif()
	target_compile_options(fiftyone-ip-intelligence-c-cov PRIVATE "--coverage")
	
	add_library(fiftyone-ip-intelligence-cxx-cov ${IPICPP_SRC} ${IPICPP_H})
	target_link_libraries(fiftyone-ip-intelligence-cxx-cov
		fiftyone-ip-intelligence-c-cov
		fiftyone-common-cxx-cov)
	target_compile_options(fiftyone-ip-intelligence-cxx-cov PRIVATE "--coverage")
	
	set_target_properties(fiftyone-ip-intelligence-c-cov fiftyone-ip-intelligence-cxx-cov
		PROPERTIES FOLDER "IP Intelligence")
endif()

set_target_properties(fiftyone-ip-intelligence-c fiftyone-ip-intelligence-cxx
	PROPERTIES FOLDER "IP Intelligence")  

# Shared library

set(EXAMPLES_BASE_SRC ${CMAKE_CURRENT_LIST_DIR}/examples/Base/ExampleBase.c)
set(EXAMPLES_BASE_H ${CMAKE_CURRENT_LIST_DIR}/examples/Base/ExampleBase.h)
file(WRITE null.cpp "")
add_library(fiftyone-ip-intelligence-complete SHARED null.cpp)
target_link_libraries(fiftyone-ip-intelligence-complete fiftyone-ip-intelligence-cxx)

set_target_properties(fiftyone-ip-intelligence-complete PROPERTIES PUBLIC_HEADER "${IPI}/ipi.h")

install(TARGETS fiftyone-ip-intelligence-complete DESTINATION fiftyone-ip-intelligence-complete
	RUNTIME DESTINATION lib
	ARCHIVE DESTINATION lib/static
	PUBLIC_HEADER DESTINATION include)

# Examples

set(EXAMPLES_FOLDER IpIntelligence)

MESSAGE("-- Looking for examples...")
foreach (api "IP-Intelligence")
	foreach (langext c cpp)
		string( TOUPPER ${langext} upperlangext)
		file(GLOB files ${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${EXAMPLES_FOLDER}/*.${langext})
		foreach(file ${files})
			string( REPLACE ".${langext}" "" name ${file})
			string( REPLACE "${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${EXAMPLES_FOLDER}/" "" name ${name})
			string( APPEND name ${upperlangext})
			string( TOLOWER ${api} lowerapi)

			if (NOT ${name} MATCHES ".*ExampleBase.*")
				if("${langext}" STREQUAL "cpp")
					MESSAGE("-- Found C++ example '${name}'")
					add_executable(${name} ${file}
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${EXAMPLES_FOLDER}/ExampleBase.cpp
					${CMAKE_CURRENT_LIST_DIR}/examples/${upperlangext}/${EXAMPLES_FOLDER}/ExampleBase.hpp
					${EXAMPLES_BASE_SRC} ${EXAMPLES_BASE_H})
					target_link_libraries(${name} fiftyone-${lowerapi}-cxx)
				elseif("${langext}" STREQUAL "c")
					MESSAGE("-- Found C example '${name}'")
					add_executable(${name} ${file}
					${EXAMPLES_BASE_SRC} ${EXAMPLES_BASE_H})
					target_link_libraries(${name} fiftyone-${lowerapi}-c)
				endif()
				set_target_properties(${name}
					PROPERTIES FOLDER
					"Examples/IP Intelligence/${upperlangext}") 
				if (MSVC)
					target_compile_options(${name} PRIVATE "/D_CRT_SECURE_NO_WARNINGS")
				endif()
			endif()
		endforeach()
	endforeach()
endforeach()

# Tests

if(BUILD_TESTING)
	set(COMMON_TEST ${CMAKE_CURRENT_LIST_DIR}/src/common-cxx/tests)
	set(IPI_TEST	${CMAKE_CURRENT_LIST_DIR}/test)

	FILE(GLOB IPI_TEST_SRC ${IPI_TEST}/*.cpp)
	FILE(GLOB IPI_TEST_H ${IPI_TEST}/*.hpp)

	add_executable(IpiTests
		${IPI_TEST_SRC} ${IPI_TEST_H}
		${COMMON_TEST}/Base.cpp
		${COMMON_TEST}/EngineTests.cpp
		${COMMON_TEST}/ExampleTests.cpp
		${EXAMPLES_BASE_SRC} ${EXAMPLES_BASE_H})
	
	if (TESTCOVERAGE_ENABLED)
		target_link_libraries(IpiTests fiftyone-ip-intelligence-cxx-cov gtest_main)
		target_link_options(IpiTests PRIVATE "-fprofile-arcs" "-ftest-coverage")
	else()
		target_link_libraries(IpiTests fiftyone-ip-intelligence-cxx gtest_main)
	endif()
	
	gtest_discover_tests(IpiTests DISCOVERY_MODE PRE_TEST)

	set_target_properties(IpiTests PROPERTIES FOLDER "Tests")

	if (MSVC)
		target_compile_options(IpiTests PRIVATE "/D_CRT_SECURE_NO_WARNINGS" "/W4" "/WX")
		target_link_options(IpiTests PRIVATE "/WX")
	else()
		target_compile_options(IpiTests PRIVATE ${COMPILE_OPTION_DEBUG})
	endif()

	if (CMAKE_COMPILER_IS_GNUCC)
		target_compile_options(IpiTests PRIVATE "-Wall" "-Werror" "-Wno-unused-variable" "-Wno-unused-result" "-Wno-unused-but-set-variable")
	endif()
endif()
