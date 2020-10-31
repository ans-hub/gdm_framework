cmake_minimum_required (VERSION 3.12)

project("gdm/framework/dx11/normal_map")

set(BIN normal_map)
set(GDM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(GDM_FRAMEWORK_DIR ${GDM_ROOT_DIR}/framework)

message("* App ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

add_definitions(-DGFX_DX_API)
set(DX_API 1)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIRS
  "."
  ${GDM_FRAMEWORK_DIR})
include_directories(${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(warnings "/W4 /WX /EHsc") # todo: W4 ???
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

set(SRC normal_map.cc)

if (WIN32)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /WX /wd4355"
      CACHE STRING "Debug compiler flags" FORCE)
  message("* App ${BIN}: set linker specific for Windows")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

message("* App ${BIN}: adding subdirectories")
add_subdirectory(${GDM_FRAMEWORK_DIR}/math/ static_libs/math)
add_subdirectory(${GDM_FRAMEWORK_DIR}/data/ static_libs/data)
add_subdirectory(${GDM_FRAMEWORK_DIR}/window/ static_libs/window)
add_subdirectory(${GDM_FRAMEWORK_DIR}/render/ static_libs/render)

message("* App ${BIN}: adding executable")
add_executable(${BIN} ${SRC})

message("* App ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} math system data window render)
target_link_libraries(${BIN} winmm d3dcompiler dxgi d3d11 dinput8 dxguid)
