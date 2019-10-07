cmake_minimum_required(VERSION 3.10) # For CXX_STANDARD 17 property on Visual Studio

# Declare printer
add_executable(printer)

# Add sources
cmutils_target_sources_folders(
  printer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Add includes
target_include_directories(
  printer PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link dependencies
target_link_libraries(
  printer PRIVATE
  dynamic_bitset
  spdlog_header_only
  nlohmann_json
  inja
  cxxopts
  git_info
)

# Link filesystem library for clang and gcc < 9.0
cmutils_define_compiler_variables()
if(COMPILER_CLANG OR (COMPILER_GCC AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)))
	target_link_libraries(printer PRIVATE stdc++fs)
endif()

# Configure compile options
cmutils_target_configure_compile_options(printer)

# Enable cmutils default warnings
cmutils_target_enable_warnings(printer)

# Build in C++17
cmutils_target_set_standard(printer CXX 17)

# Set static runtime
cmutils_target_set_runtime(printer STATIC)

# Enable sanitizers in debug
cmutils_target_enable_sanitizers(
  printer
  ADDRESS LEAK UNDEFINED MEMORY
  DEBUG
)

# Enable debug logs
cmutils_target_add_compile_definition(
  printer
  "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG"
)

# Set target IDE folder
cmutils_target_set_ide_folder(printer "USCP")

# Group sources for IDEs
cmutils_target_source_group(printer "${CMAKE_CURRENT_SOURCE_DIR}")

# Visual studio config
get_target_property(printer_binary_folder printer RUNTIME_OUTPUT_DIRECTORY)
set_target_properties(
  printer PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${printer_binary_folder}"
)

# Use ccache
cmutils_target_use_ccache(printer)

# Generate format target
cmutils_target_generate_clang_format(printer)
