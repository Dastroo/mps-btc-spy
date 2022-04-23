cmake_minimum_required(VERSION 3.16)
project(mps-btc-spy)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")

add_executable(${PROJECT_NAME} main.cpp
        include/BtcSpy.h
        include/Timer.h
        include/Mempool.h src/Mempool.cpp
        include/MsNode.h src/MsNode.cpp
        include/BdfNode.h src/BdfNode.cpp)

target_link_libraries(${PROJECT_NAME} curlpp curl
        jsoncpp
        log-helper
        mps_utils my_utils)

if (UNIX AND NOT APPLE)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME}
            DESTINATION ${CMAKE_INSTALL_BINDIR})

    # make uninstall
    add_custom_target("uninstall" COMMENT "Uninstall installed files")
    add_custom_command(
            TARGET "uninstall"
            POST_BUILD
            COMMENT "Uninstall files with install_manifest.txt"
            COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in install_manifest.txt to be uninstalled!)
endif ()