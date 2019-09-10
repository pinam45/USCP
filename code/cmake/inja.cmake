message(STATUS "Configuring inja")

get_filename_component(INJA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/inja ABSOLUTE)

# Submodule check
cmutils_directory_is_empty(is_empty "${INJA_DIR}")
if(is_empty)
	message(FATAL_ERROR "inja dependency is missing, maybe you didn't pull the git submodules")
endif()

# Include inja
set(INJA_USE_EMBEDDED_JSON OFF CACHE INTERNAL "")
set(INJA_INSTALL OFF CACHE INTERNAL "")
set(INJA_EXPORT OFF CACHE INTERNAL "")
set(BUILD_TESTING OFF CACHE INTERNAL "")
set(BUILD_BENCHMARK OFF CACHE INTERNAL "")
set(COVERALLS OFF CACHE INTERNAL "")
add_subdirectory(${INJA_DIR})
if(NOT TARGET inja)
	message(FATAL_ERROR "inja target is missing")
endif()

# Disable warnings on inja headers
get_target_property(inja_include_directories inja INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(inja PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
target_include_directories(inja SYSTEM INTERFACE ${inja_include_directories})

message(STATUS "Configuring inja - Done")
