# used to automatically include and build dependencies
include(FetchContent)

# i'd rather stay on 3.11 than bumping to 3.14 just for this macro
if(${CMAKE_VERSION} VERSION_LESS "3.14.0")
	message("Adding FetchContent_MakeAvailable")
	macro(FetchContent_MakeAvailable)
		foreach(contentName IN ITEMS ${ARGV})
			string(TOLOWER ${contentName} contentNameLower)
			FetchContent_GetProperties(${contentName})
			if(NOT ${contentNameLower}_POPULATED)
				FetchContent_Populate(${contentName})
				if(EXISTS ${${contentNameLower}_SOURCE_DIR}/CMakeLists.txt)
					add_subdirectory(${${contentNameLower}_SOURCE_DIR}
						${${contentNameLower}_BINARY_DIR})
				endif()
			endif()
		endforeach()
	endmacro()
endif()

FetchContent_Declare(
    minIni
    GIT_REPOSITORY https://github.com/compuphase/minIni.git
    GIT_TAG master
)

FetchContent_MakeAvailable(minIni)

# the below don't have a cmake build system so we will just manually include them
# TODO: review the spdlog dependency. should we be pulling spdlog from git or apt-get install?
set(EXTERN_INCLUDE_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${FETCHCONTENT_BASE_DIR}/minini-src/dev"
    "${CMAKE_CURRENT_SOURCE_DIR}/ValveFileVDF"
    "${CMAKE_CURRENT_SOURCE_DIR}/miniz"
    PARENT_SCOPE
)

# the below don't have a cmake build system so we will just manually include them
set(EXTERN_SOURCE_FILES
    "${FETCHCONTENT_BASE_DIR}/minini-src/dev/minIni.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/miniz/miniz.c"
    PARENT_SCOPE
)