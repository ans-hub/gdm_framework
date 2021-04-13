cmake_minimum_required (VERSION 3.10)

project("gdm/framework/engine")

set(BIN engine)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)

include_directories(
    "engine/"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIRS}
)

set(SRC_FILES
    camera_eul.cc
    gui_manager.cc
    gpu_streamer.cc
    debug_draw.cc
    cfg_dispatcher.cc)

# -- Libs

message("* Lib ${BIN}: adding libs")

# -- Executable --

message("* Lib ${BIN}: adding executable")
add_library(${BIN} STATIC ${SRC_FILES})

# -- Link --

message("* Lib ${BIN}: linking 3rd libraries")
