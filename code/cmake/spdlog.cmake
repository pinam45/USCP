message(STATUS "Configuring spdlog")

get_filename_component(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extlibs/spdlog ABSOLUTE)

# Submodule check
cmutils_directory_is_empty(is_empty "${SPDLOG_DIR}")
if(is_empty)
	message(FATAL_ERROR "Spdlog dependency is missing, maybe you didn't pull the git submodules")
endif()

# Include spdlog
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory(${SPDLOG_DIR})
if(NOT TARGET spdlog_header_only)
	message(FATAL_ERROR "spdlog_header_only target is missing")
endif()

# Set target IDE folder
cmutils_target_set_ide_folder(spdlog "extlibs/spdlog")
if(TARGET spdlog_headers_for_ide)
	cmutils_target_set_ide_folder(spdlog_headers_for_ide "extlibs/spdlog")
endif()

# Disable warnings on spdlog headers
get_target_property(spdlog_include_directories spdlog_header_only INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(spdlog_header_only PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
target_include_directories(spdlog_header_only SYSTEM INTERFACE ${spdlog_include_directories})

message(STATUS "Configuring spdlog - Done")
