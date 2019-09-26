message(STATUS "Configuring cxxopts")

get_filename_component(CXXOPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/cxxopts ABSOLUTE)

# Submodule check
cmutils_directory_is_empty(is_empty "${CXXOPTS_DIR}")
if(is_empty)
	message(FATAL_ERROR "cxxopts dependency is missing, maybe you didn't pull the git submodules")
endif()

# Include cxxopts
set(CXXOPTS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(CXXOPTS_BUILD_TESTS OFF CACHE INTERNAL "")
set(CXXOPTS_ENABLE_INSTALL OFF CACHE INTERNAL "")
add_subdirectory(${CXXOPTS_DIR})
if(NOT TARGET cxxopts)
	message(FATAL_ERROR "cxxopts target is missing")
endif()

# Disable warnings on cxxopts headers
get_target_property(cxxopts_include_directories cxxopts INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(cxxopts PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
target_include_directories(cxxopts SYSTEM INTERFACE ${cxxopts_include_directories})

message(STATUS "Configuring cxxopts - Done")
