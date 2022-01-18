cmake_minimum_required(VERSION 3.11)

project(vsg-siege
    VERSION 0.0.0
    DESCRIPTION "Open Source implementation of Dungeon Siege"
)

# add custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# TODO: eventually these should be switched to off
OPTION (SIEGE_BUILD_VSG_EXAMPLES "Build mimicked vsgExamples - useful for PRs that don't require Dungeon Siege content" ON)
OPTION (SIEGE_BUILD_TEST_STATES "Build test states" ON)
OPTION (SIEGE_REGENERATE_FUEL_PARSER "Regenerate fuel parser source files" OFF)

# globally set 17 as the standard so imported modules get the flag as well
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(extern)

# find the vsg
find_package(vsg REQUIRED)

# not sure if this is still required or not
if (UNIX)
    find_package(PkgConfig)
    pkg_check_modules(XDGBASEDIR REQUIRED libxdg-basedir)
endif(UNIX)

if (SIEGE_BUILD_VSG_EXAMPLES)
    set (SIEGE_VSG_EXAMPLES_SOURCES
        src/state/vsgExamples/vsgExamplesDraw.cpp
    )
endif()

if (SIEGE_BUILD_TEST_STATES)
    set (SIEGE_TEST_STATES_SOURCE
        src/state/test/SiegeNodeTestState.cpp
        src/state/test/RegionTestState.cpp
        src/state/test/FullMapTestState.cpp
        src/state/test/RegionStitchTestState.cpp
        src/state/test/ProfileLoadingState.cpp
        src/state/test/ASPMeshTestState.cpp
    )
endif()

if(SIEGE_REGENERATE_FUEL_PARSER)
    # these are required due to having exes associated with them
    find_package(BISON REQUIRED)
    find_package(RE2C REQUIRED)

    bison_target(FuelParser "${CMAKE_SOURCE_DIR}/src/gas/FuelParser.y" "${CMAKE_CURRENT_LIST_DIR}/src/gas/FuelParser.cpp" COMPILE_FLAGS "${YACC_FLAGS} --no-lines --defines=${CMAKE_CURRENT_LIST_DIR}/src/gas/FuelParser.hpp")
    re2c_target(NAME FuelScanner INPUT "${CMAKE_CURRENT_LIST_DIR}/src/gas/FuelScanner.r2c" OUTPUT "${CMAKE_CURRENT_LIST_DIR}/src/gas/FuelScanner.cpp" OPTIONS "--no-debug-info")
endif()

set (SOURCES
    # config
    src/cfg/WritableConfig.cpp
    src/cfg/ArgsConfig.cpp
    src/cfg/UserConfig.cpp
    src/cfg/RegistryConfig.cpp
    src/cfg/SteamConfig.cpp

    # state
    src/state/IGameState.cpp
    src/state/GameStateMgr.cpp
    src/state/InitState.cpp
    ${SIEGE_TEST_STATES_SOURCE}
    "${SIEGE_VSG_EXAMPLES_SOURCES}"

    # io
    src/io/BinaryReader.cpp
    src/io/StringTool.cpp
    src/io/NamingKeyMap.cpp
    src/io/LocalFileSys.cpp
    src/io/tank/TankFile.cpp
    src/io/tank/TankFileReader.cpp
    src/io/TankFileSys.cpp
    
    # gas
    src/gas/Fuel.cpp
    src/gas/FuelParser.cpp
    src/gas/FuelScanner.cpp

    # world
    src/world/WorldMapData.cpp
    src/world/SiegeNode.cpp
    src/world/Aspect.cpp
    src/world/Region.cpp

    # vsg
    src/vsg/ReaderWriterRAW.cpp
    src/vsg/ReaderWriterSNO.cpp
    src/vsg/ReaderWriterASP.cpp
    src/vsg/ReaderWriterSiegeNodeList.cpp
    src/vsg/ReaderWriterRegion.cpp
)

message (STATUS "extern_source_files: " ${EXTERN_SOURCE_FILES})
add_executable (vsg-siege ${EXTERN_SOURCE_FILES} src/main.cpp src/Game.cpp src/ContentDb.cpp src/SiegeVisitor.cpp ${SOURCES})
target_link_libraries (vsg-siege PRIVATE vsg::vsg "$<$<CXX_COMPILER_ID:GNU>:stdc++fs;${XDGBASEDIR_LIBRARIES}>")
target_include_directories(vsg-siege PUBLIC src ${EXTERN_INCLUDE_PATHS})

if (SIEGE_BUILD_VSG_EXAMPLES)
    target_compile_definitions(vsg-siege PRIVATE SIEGE_VSG_EXAMPLES_ENABLED)
endif()

if (SIEGE_BUILD_TEST_STATES)
    target_compile_definitions(vsg-siege PRIVATE SIEGE_TEST_STATES_ENABLED)
endif()

if(MSVC)
    target_compile_options(vsg-siege PRIVATE "/MP")

    source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${SOURCES})
endif()

vsg_add_target_clang_format(
    FILES
        ${CMAKE_SOURCE_DIR}/src/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*/*.hpp
        ${CMAKE_SOURCE_DIR}/src/*/*.cpp
)
