cmake_minimum_required (VERSION 3.10)

# -- Project initials --

project("gdm/3rdparty/catch")
set(BIN catch)

# --

set(SRC_FILES
    catch.cc)

# -- Build --

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

add_library(${BIN} STATIC ${SRC_FILES})
