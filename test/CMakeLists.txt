cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(memheap LANGUAGES CXX)

# -Dmemheap_build_tests=ON flag with cmake.
option(${PROJECT_NAME}_build_tests "Build unit tests." OFF)

message( STATUS "Generator - ${CMAKE_GENERATOR}")
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

if( APPLE )
	set( CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ -Wall" )
else()
	set( CMAKE_CXX_FLAGS "-std=c++11 -Wall -pthread" )
endif()

include_directories("include")

file(GLOB src src/*.cpp src/*.h include/${PROJECT_NAME}/*.h)

add_library(${PROJECT_NAME} ${src})

# Attach header directory information
# to the targets for when we are part of a parent build (ie being pulled
# in via add_subdirectory() rather than being a standalone build).
target_include_directories(${PROJECT_NAME} INTERFACE "include")

# Install rules
install(TARGETS ${PROJECT_NAME} 
		LIBRARY DESTINATION "lib"
		ARCHIVE DESTINATION "lib"
		RUNTIME DESTINATION "bin"
	)
install(DIRECTORY include/${PROJECT_NAME} DESTINATION include) 

# Unit tests
if (${PROJECT_NAME}_build_tests)
	find_package(GTest REQUIRED)
	include_directories(${GTEST_INCLUDE_DIRS})

	enable_testing()

	add_executable(heaptest test/heaptest.cpp)
	target_link_libraries(heaptest ${PROJECT_NAME} ${GTEST_LIBRARIES})

	add_test(heaptest heaptest)

	add_executable(benchmark test/benchmark.cpp)
	target_link_libraries(benchmark ${PROJECT_NAME})

	add_executable(stdbenchmark test/stdbenchmark.cpp)
	target_link_libraries(stdbenchmark ${PROJECT_NAME})
endif()
