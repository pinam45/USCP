cmake_minimum_required(VERSION 3.10) # For CXX_STANDARD 17 property on Visual Studio

# Options
option(USCP_RWLS_LOW_MEMORY_FOOTPRINT "Enable RWLS low memory footprint (slower)" OFF)

# Declare solver
add_executable(solver)

# Add sources
cmutils_target_sources_folders(
  solver PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Add includes
target_include_directories(
  solver PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link dependencies
target_link_libraries(
  solver PRIVATE
  common
  dynamic_bitset
  spdlog_header_only
  nlohmann_json
  inja
  cxxopts
  git_info
)

# Low memory footprint
if(USCP_RWLS_LOW_MEMORY_FOOTPRINT)
	cmutils_target_add_compile_definition(solver USCP_RWLS_LOW_MEMORY_FOOTPRINT)
	message(STATUS "Enabled RWLS low memory footprint")
endif()

# Link OpenMP in Release if found
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
	target_link_libraries(solver PRIVATE $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:OpenMP::OpenMP_CXX>)
	message(STATUS "OpenMP found, it will be used")
else()
	message(WARNING "OpenMP not found, it will not be used")
endif()

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
#cmutils_target_set_runtime(solver STATIC)

# Enable sanitizers in debug
cmutils_target_enable_sanitizers(
  solver
  ADDRESS LEAK UNDEFINED MEMORY
  RELWITHDEBINFO DEBUG
)

# Enable debug logs
cmutils_target_add_compile_definition(
  solver
  "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG"
)

# Set target IDE folder
cmutils_target_set_ide_folder(solver "USCP")

# Group sources for IDEs
cmutils_target_source_group(solver "${CMAKE_CURRENT_SOURCE_DIR}")

# Visual studio config
get_target_property(solver_binary_folder solver RUNTIME_OUTPUT_DIRECTORY)
set_target_properties(
  solver PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${solver_binary_folder}"
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
	  "-flto"
	)

	foreach(flag ${flags})
		cmutils_target_add_cxx_compiler_flag(solver ${flag} RELEASE RELWITHDEBINFO)
		cmutils_target_add_cxx_linker_flag(solver ${flag} RELEASE RELWITHDEBINFO)
	endforeach()
endif()

if(DEFINED PROFILE_GUIDED_OPTIMIZATION)
	if(${PROFILE_GUIDED_OPTIMIZATION} STREQUAL GENERATE)
		target_compile_options(solver PRIVATE "-fprofile-generate=${CMAKE_CURRENT_BINARY_DIR}/pgo")
		if(COMMAND target_link_options)
			target_link_options(solver PRIVATE "-fprofile-generate=${CMAKE_CURRENT_BINARY_DIR}/pgo")
		else()
			# old CMake version: use ugly workaround
			__cmutils_target_link_flag_property(solver "-fprofile-generate=${CMAKE_CURRENT_BINARY_DIR}/pgo")
		endif()
		message(STATUS "solver: Profile Guided Optimization generation enabled")
	elseif(${PROFILE_GUIDED_OPTIMIZATION} STREQUAL USE)
		target_compile_options(solver PRIVATE "-fprofile-use=${CMAKE_CURRENT_BINARY_DIR}/pgo")
		if(COMMAND target_link_options)
			target_link_options(solver PRIVATE "-fprofile-use=${CMAKE_CURRENT_BINARY_DIR}/pgo")
		else()
			# old CMake version: use ugly workaround
			__cmutils_target_link_flag_property(solver "-fprofile-use=${CMAKE_CURRENT_BINARY_DIR}/pgo")
		endif()
		message(STATUS "solver: Profile Guided Optimization usage enabled")
	else()
		message(STATUS "solver: invalid PROFILE_GUIDED_OPTIMIZATION value, use GENERATE or USE")
	endif()
else()
	message(STATUS "solver: Profile Guided Optimization usage disabled")
endif()
