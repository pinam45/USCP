cmake_minimum_required(VERSION 3.10) # For CXX_STANDARD 17 property on Visual Studio

# Declare common
add_library(common STATIC)

# Add sources
cmutils_target_sources_folders(
  common PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Add includes
target_include_directories(
  common PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link dependencies
target_link_libraries(
  common PUBLIC
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
	target_link_libraries(common PRIVATE stdc++fs)
endif()

# Configure compile options
cmutils_target_configure_compile_options(common)

# Enable cmutils default warnings
cmutils_target_enable_warnings(common)

# Build in C++17
cmutils_target_set_standard(common CXX 17)

# Enable debug logs
cmutils_target_add_compile_definition(
  common
  "SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG"
)

# Set target IDE folder
cmutils_target_set_ide_folder(common "USCP")

# Group sources for IDEs
cmutils_target_source_group(common "${CMAKE_CURRENT_SOURCE_DIR}")

# Use ccache
cmutils_target_use_ccache(common)

# Generate format target
cmutils_target_generate_clang_format(common)
