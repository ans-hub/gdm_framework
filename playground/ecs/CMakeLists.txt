cmake_minimum_required (VERSION 3.12)

project("gdm/example/ecs/cubes")

set(BIN cubes)
set(GDM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(GDM_FRAMEWORK_DIR ${GDM_ROOT_DIR}/framework)

message("* App ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

set(CMAKE_CXX_STANDARD 20)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(warnings "/W4 /WX /EHsc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /MP /WX /wd4355 /wd4267 /Zc:preprocessor /Wv:18"
    CACHE STRING "Debug compiler flags" FORCE)
endif()

set(INCLUDE_DIRS
  "."
  ${GDM_FRAMEWORK_DIR})
include_directories(${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

set(SRC cubes.cc render.ecs.cc)

if (WIN32)
  message("* App ${BIN}: set linker specific for Windows")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

message("* App ${BIN}: adding subdirectories")
add_subdirectory(${GDM_FRAMEWORK_DIR}/ecs/ static_libs/ecs)
add_subdirectory(${GDM_FRAMEWORK_DIR}/math/ static_libs/math)
add_subdirectory(${GDM_FRAMEWORK_DIR}/data/ static_libs/data)
add_subdirectory(${GDM_FRAMEWORK_DIR}/window/ static_libs/window)
add_subdirectory(${GDM_FRAMEWORK_DIR}/render/ static_libs/render)
add_subdirectory(${GDM_FRAMEWORK_DIR}/threads/ static_libs/threads)

message("* App ${BIN}: adding executable")
add_executable(${BIN} ${SRC})

message("* App ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ecs math system data window render threads)
target_link_libraries(${BIN} winmm d3dcompiler dxgi d3d11 dinput8 dxguid)
