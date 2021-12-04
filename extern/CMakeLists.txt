# used to automatically include and build dependencies
include(FetchContent)

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
    PARENT_SCOPE
)