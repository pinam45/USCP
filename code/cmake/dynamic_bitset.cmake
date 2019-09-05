message(STATUS "Configuring dynamic_bitset")

get_filename_component(DYNAMIC_BITSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/dynamic_bitset ABSOLUTE)

# Submodule check
cmutils_directory_is_empty(is_empty "${DYNAMIC_BITSET_DIR}")
if(is_empty)
	message(FATAL_ERROR "dynamic_bitset dependency is missing, maybe you didn't pull the git submodules")
endif()

add_subdirectory(${DYNAMIC_BITSET_DIR})

# Message
message(STATUS "Configuring dynamic_bitset - Done")
