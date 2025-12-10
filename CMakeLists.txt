cmake_minimum_required(VERSION 3.15)
project(memheap LANGUAGES CXX)

# -Dmemheap_build_tests=ON flag with cmake.
option(${PROJECT_NAME}_build_tests "Build unit tests." OFF)

message( STATUS "Generator - ${CMAKE_GENERATOR}")
message( STATUS "Build type - ${CMAKE_BUILD_TYPE}")

if (${PROJECT_NAME}_build_tests)
	find_package(GTest CONFIG REQUIRED)
endif()

include(GNUInstallDirs)

if( APPLE )
	set( CMAKE_CXX_FLAGS "-std=c++17 -stdlib=libc++ -Wall" )
else()
	set( CMAKE_CXX_FLAGS "-std=c++17 -Wall -pthread" )
endif()

add_link_options("-lstdc++")

include_directories("include")

file(GLOB src src/*.cpp src/*.h include/${PROJECT_NAME}/*.h)

add_library(${PROJECT_NAME} ${src})

# Attach header directory information
# to the targets for when we are part of a parent build (ie being pulled
# in via add_subdirectory() rather than being a standalone build).
target_include_directories(${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Install rules
install(TARGETS ${PROJECT_NAME}
		EXPORT memheapTargets
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
install(DIRECTORY include/${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install the export set for use with the install-tree
install(EXPORT memheapTargets
	FILE memheapTargets.cmake
	NAMESPACE memheap::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memheap
)

# Create and install the config file
include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/memheapConfig.cmake.in
	"${CMAKE_CURRENT_BINARY_DIR}/memheapConfig.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memheap
)

# Generate the version file for the config file
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/memheapConfigVersion.cmake"
	VERSION "1.0.0"
	COMPATIBILITY AnyNewerVersion
)

# Install the config and version files
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/memheapConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/memheapConfigVersion.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/memheap
) 

add_executable(benchmark test/benchmark.cpp)
target_link_libraries(benchmark ${PROJECT_NAME})

# Unit tests
if (${PROJECT_NAME}_build_tests)
	enable_testing()

	file(GLOB src test/heaptest.cpp)

	add_executable(heaptest EXCLUDE_FROM_ALL ${src})

	target_link_libraries(
		heaptest
		GTest::gtest_main
		memheap
	)

	include(GoogleTest)
	gtest_discover_tests(heaptest)
endif()
