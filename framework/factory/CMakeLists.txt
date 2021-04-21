cmake_minimum_required (VERSION 3.10)

project("gdm/framework/factory")

set(BIN factory)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)

include_directories(
    "factory/"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIRS}
)

set(SRC_FILES
    data_factory.cc
    image_factory.cc
    mesh_factory.cc
    material_factory.cc
    texture_factory.cc
    model_factory.cc)

# -- Executable --

message("* Lib ${BIN}: adding executable")
add_library(${BIN} STATIC ${SRC_FILES})
