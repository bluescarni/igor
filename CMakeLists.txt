# NOTE: C++20 supported since version 3.12.
cmake_minimum_required(VERSION 3.12.0)

# Set default build type to "Release".
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
	FORCE)
endif()

project(igor VERSION 1.0.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacma")

message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "igor version: ${igor_VERSION}")

# The build options.
option(IGOR_BUILD_TESTS "Build unit tests." OFF)

include(YACMACompilerLinkerSettings)

# Assemble the flags.
set(IGOR_CXX_FLAGS_DEBUG ${YACMA_CXX_FLAGS} ${YACMA_CXX_FLAGS_DEBUG})
set(IGOR_CXX_FLAGS_RELEASE ${YACMA_CXX_FLAGS})
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin" AND YACMA_COMPILER_IS_CLANGXX)
  message(STATUS "Clang compiler on OSX detected, setting the standard library to 'libc++'.")
  list(APPEND IGOR_CXX_FLAGS_DEBUG "-stdlib=libc++")
  list(APPEND IGOR_CXX_FLAGS_RELEASE "-stdlib=libc++")
endif()
if(YACMA_COMPILER_IS_MSVC)
  # Enable the bigobj option.
  list(APPEND IGOR_CXX_FLAGS_DEBUG "/bigobj")
  list(APPEND IGOR_CXX_FLAGS_RELEASE "/bigobj")
endif()
if(MINGW)
  # In MinGW some tests generate big object files.
  list(APPEND IGOR_CXX_FLAGS_DEBUG "-Wa,-mbig-obj")
  list(APPEND IGOR_CXX_FLAGS_RELEASE "-Wa,-mbig-obj")
endif()

# Setup of the igor interface library.
add_library(igor INTERFACE)

target_include_directories(igor INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>)

# Installation.
# Setup of the export.
install(TARGETS igor EXPORT igor_export)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/igor-config.cmake" DESTINATION "lib/cmake/igor")
install(EXPORT igor_export NAMESPACE igor:: DESTINATION lib/cmake/igor)
# Take care of versioning.
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/igor-config-version.cmake" VERSION ${igor_VERSION}
	COMPATIBILITY SameMajorVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/igor-config-version.cmake" DESTINATION "lib/cmake/igor")

# Installation of the header files.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/igor" DESTINATION include)

if(IGOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
