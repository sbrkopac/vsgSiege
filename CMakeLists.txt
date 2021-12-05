cmake_minimum_required(VERSION 3.7)

project(vsg-siege
    VERSION 0.0.0
    DESCRIPTION "Open Source implementation of Dungeon Siege"
    LANGUAGES CXX
)

# TODO: eventually this should be switched to off
OPTION (SIEGE_BUILD_VSG_EXAMPLES "Build mimicked vsgExamples - useful for PRs that don't require Dungeon Siege content" ON)

# find the vsg
find_package(vsg REQUIRED)

set(CMAKE_CXX_STANDARD 17)

if (SIEGE_BUILD_VSG_EXAMPLES)
    set (SIEGE_VSG_EXAMPLES_SOURCES
        src/state/vsgExamples/vsgExamplesDraw.cpp
    )
endif()

set (SOURCES
    # config
    src/cfg/WritableConfig.cpp
    src/cfg/ArgsConfig.cpp
    src/cfg/UserConfig.cpp
    src/cfg/RegistryConfig.cpp
    src/cfg/SteamConfig.cpp

    # state
    src/state/GameStateMgr.cpp
    src/state/InitState.cpp
    src/state/SiegeNodeTestState.cpp
    src/state/RegionTestState.cpp
    src/state/FullMapTestState.cpp
    src/state/RegionStitchTestState.cpp
    src/state/ProfileLoadingState.cpp
    "${SIEGE_VSG_EXAMPLES_SOURCES}"

    # io
    src/io/BinaryReader.cpp
    src/io/StringTool.cpp
    src/io/NamingKeyMap.cpp
    src/io/Fuel.cpp
    src/io/FuelParser.cpp
    src/io/FuelScanner.cpp
    src/io/LocalFileSys.cpp

    # world
    src/world/WorldMapData.cpp
    src/world/SiegeNode.cpp

    # vsg
    src/vsg/ReaderWriterRAW.cpp
    src/vsg/ReaderWriterSNO.cpp
    src/vsg/ReaderWriterSiegeNodeList.cpp
    src/vsg/ReaderWriterRegion.cpp
)

add_executable (vsg-siege src/main.cpp src/Game.cpp ${SOURCES})
target_link_libraries (vsg-siege vsg::vsg)
target_include_directories(vsg-siege PUBLIC src extern)

if (SIEGE_BUILD_VSG_EXAMPLES)
    target_compile_definitions(vsg-siege PRIVATE SIEGE_VSG_EXAMPLES_ENABLED)
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
