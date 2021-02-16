cmake_minimum_required (VERSION 3.10)

project("gdm/framework/math")
set(BIN math)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

include_directories(
    "math/"
    ${INCLUDE_DIRS}
)

set(SRC_FILES
    line2.cc
    line3.cc
    polygon2.cc
    segment2.cc
    intersection.cc)

add_library(${BIN} STATIC ${SRC_FILES})
