cmake_minimum_required (VERSION 3.10)

project("gdm/framework/data")

set(BIN data)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)

include_directories(
    "data/"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIRS}
)

set(SRC_FILES
    data_factory.cc
    image_factory.cc
    mesh_factory.cc
    material_factory.cc
    texture_factory.cc
    model_factory.cc
    image_helpers.cc
    model_helpers.cc
    cfg_loader.cc
    abstract_image.cc
    bmp_loader.cc
    model_loader.cc
    ply_loader.cc
    png_loader.cc
    tga_loader.cc)

# -- Libs

message("* Lib ${BIN}: adding libs")
add_subdirectory(${GDM_FRAMEWORK_DIR}/memory/ gdm_libs/memory)

# -- Executable --

message("* Lib ${BIN}: adding executable")
add_library(${BIN} STATIC ${SRC_FILES})

# -- Link --

message("* Lib ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ${CMAKE_THREAD_LIBS_INIT} memory)
