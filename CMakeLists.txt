cmake_minimum_required(VERSION 3.11)

project(vsg-siege
    VERSION 0.0.0
    DESCRIPTION "Open Source implementation of Dungeon Siege"
)

# globally set 17 as the standard so imported modules get the flag as well
set(CMAKE_CXX_STANDARD 17)

# find the vsg
find_package(vsg REQUIRED)

# not sure if this is still required or not
if (UNIX)
    find_package(PkgConfig)
    pkg_check_modules(XDGBASEDIR REQUIRED libxdg-basedir)
endif(UNIX)

add_subdirectory(extern)

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

message (STATUS "extern_source_files: " ${EXTERN_SOURCE_FILES})
add_executable (vsg-siege ${EXTERN_SOURCE_FILES} src/main.cpp src/Game.cpp ${SOURCES})
target_link_libraries (vsg-siege PRIVATE vsg::vsg "$<$<CXX_COMPILER_ID:GNU>:stdc++fs;${XDGBASEDIR_LIBRARIES}>")
target_include_directories(vsg-siege PUBLIC src ${EXTERN_INCLUDE_PATHS})

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
