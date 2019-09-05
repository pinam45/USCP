cmake_minimum_required(VERSION 3.10) # For CXX_STANDARD 17 property on Visual Studio

# Project declaration
project(USCP)

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

# Libraries
include(cmake/dynamic_bitset.cmake)
include(cmake/spdlog.cmake)
include(cmake/json.cmake)

# Declare solver
add_executable(solver)

# Add sources
cmutils_target_sources_folders(
  solver PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/solver/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/solver/include"
)

# Add includes
target_include_directories(
  solver PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/solver/include"
)

# Link dependencies
target_link_libraries(
  solver PRIVATE
  dynamic_bitset
  spdlog
  nlohmann_json
)

# Link threads library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(solver PRIVATE Threads::Threads)

# Link filesystem library for clang and gcc < 9.0
cmutils_define_compiler_variables()
if(COMPILER_CLANG OR (COMPILER_GCC AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)))
	target_link_libraries(solver PRIVATE stdc++fs)
endif()

# Configure compile options
cmutils_target_configure_compile_options(solver)

# Enable cmutils default warnings
cmutils_target_enable_warnings(solver)

# Build in C++17
cmutils_target_set_standard(solver CXX 17)

# Set static runtime
cmutils_target_set_runtime(solver STATIC)

# Enable sanitizers in debug
cmutils_target_enable_sanitizers(
  solver
  ADDRESS LEAK UNDEFINED MEMORY
  DEBUG
)

# Enable debug logs
cmutils_target_add_compile_definition(solver "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG" RELWITHDEBINFO DEBUG)

# Set target IDE folder
cmutils_target_set_ide_folder(solver "solver")

# Group sources for IDEs
cmutils_target_source_group(solver "${CMAKE_CURRENT_SOURCE_DIR}")

# Visual studio config
get_target_property(solver_binary_folder solver RUNTIME_OUTPUT_DIRECTORY)
set_target_properties(
  solver PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${solver_binary_folder}"
)
set_property(
  DIRECTORY
  ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
  VS_STARTUP_PROJECT solver
)

# Resources
cmutils_configure_folder(
  "${CMAKE_CURRENT_SOURCE_DIR}/resources"
  "${solver_binary_folder}/resources"
  COPYONLY
)

# Use ccache
cmutils_target_use_ccache(solver)

# Generate format target
cmutils_target_generate_clang_format(solver)

# special optimisation flags
cmutils_define_compiler_variables()
if(COMPILER_GCC OR COMPILER_CLANG)
	set(flags
	  # https://gcc.gnu.org/onlinedocs/gcc-4.5.3/gcc/i386-and-x86_002d64-Options.html
	  "-march=native"
	  "-mtune=native"
	  # https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#Optimize-Options
	  "-fmodulo-sched"
	  "-fmodulo-sched-allow-regmoves"
	  "-fgcse-sm"
	  "-fgcse-las"
	  "-fgcse-after-reload"
	  "-fdevirtualize-speculatively"
	  "-fdevirtualize-at-ltrans"
	  "-floop-nest-optimize"
	  "-floop-parallelize-all"
	  "-fsplit-loops"
	  "-funswitch-loops"
	  "-floop-unroll-and-jam"
	)

	foreach(flag ${flags})
		cmutils_target_add_cxx_compiler_flag(solver ${flag} RELEASE RELWITHDEBINFO)
		cmutils_target_add_cxx_linker_flag(solver ${flag} RELEASE RELWITHDEBINFO)
	endforeach()
endif()

# Verbose makefile
#set(CMAKE_VERBOSE_MAKEFILE ON)
