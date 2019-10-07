cmake_minimum_required(VERSION 3.10) # For CXX_STANDARD 17 property on Visual Studio

# Project declaration
project(
  USCP
  DESCRIPTION "Unicost Set Cover Problem"
  HOMEPAGE_URL "https://github.com/pinam45/USCP"
  LANGUAGES CXX
)

# By default build in Release mode
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type: Release, RelWithDebInfo or Debug" FORCE)
endif()

# CMake utilities
get_filename_component(CMUTILS_PATH "cmake/CMutils/cmutils.cmake" ABSOLUTE)
if(EXISTS "${CMUTILS_PATH}")
	include(${CMUTILS_PATH})
else()
	message(FATAL_ERROR "CMutils is missing, maybe you didn't pull the git submodules")
endif()

# Global config
cmutils_disable_in_sources_build()
cmutils_enable_ide_folders()
cmutils_set_classic_output_folders()

# Configure libraries
include(cmake/dynamic_bitset.cmake)
include(cmake/spdlog.cmake)
include(cmake/json.cmake)
include(cmake/inja.cmake)
include(cmake/cxxopts.cmake)

# Generate git information target
cmutils_generate_git_info_target(git_info ${CMAKE_CURRENT_SOURCE_DIR})

# Add solver
add_subdirectory(solver)

# Add printer
add_subdirectory(printer)

# Visual studio config
set_property(
  DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
  VS_STARTUP_PROJECT solver
)

# Resources
cmutils_configure_folder(
  "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources"
  COPYONLY
)

# Verbose makefile
#set(CMAKE_VERBOSE_MAKEFILE ON)
