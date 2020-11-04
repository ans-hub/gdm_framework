cmake_minimum_required (VERSION 3.12)

project("gdm/framework/vk/deferred")

set(BIN deferred)
set(GDM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(GDM_FRAMEWORK_DIR ${GDM_ROOT_DIR}/framework)
set(GDM_LIBVK_DIR ${GDM_ROOT_DIR}/3rdparty/vulkan_sdk/1.2.148.1)
set(GDM_LIBDXC_DIR ${GDM_ROOT_DIR}/3rdparty/dxc)

message("* App ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

add_definitions(-DGFX_VK_API)
set(VK_API 1)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIRS
  "."
  ${GDM_ROOT_DIR}
  ${GDM_FRAMEWORK_DIR}
  ${GDM_LIBVK_DIR}/Include
  ${GDM_LIBDXC_DIR}/include)
include_directories(${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  set(warnings "/W4 /WX /EHsc") # todo: W4 ???
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

set(SRC deferred.cc
        scene_manager.cc)

if (WIN32)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /WX /wd4355"
      CACHE STRING "Debug compiler flags" FORCE)
  message("* App ${BIN}: set linker specific for Windows")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

message("* App ${BIN}: find packages")
set(Vulkan_LIBRARY ${GDM_LIBVK_DIR}/Lib/vulkan-1.lib)
set(Vulkan_INCLUDE_DIR ${GDM_LIBVK_DIR}/Include)
find_package(Vulkan REQUIRED FATAL_ERROR)

message(${Vulkan_LIBRARY})
message(${Vulkan_INCLUDE_DIR})

message("* App ${BIN}: adding subdirectories")
add_subdirectory(${GDM_FRAMEWORK_DIR}/math/ static_libs/math)
add_subdirectory(${GDM_FRAMEWORK_DIR}/data/ static_libs/data)
add_subdirectory(${GDM_FRAMEWORK_DIR}/window/ static_libs/window)
add_subdirectory(${GDM_FRAMEWORK_DIR}/render/ static_libs/render)

message("* App ${BIN}: adding executable")
add_executable(${BIN} ${SRC})

message("* App ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} math system data window render)
target_link_libraries(${BIN} winmm dxgi dinput8 dxguid)
target_link_libraries(${BIN} ${Vulkan_LIBRARY})

message("* App ${BIN}: copy dlls")
add_custom_command(TARGET ${BIN} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${GDM_LIBDXC_DIR}/bin/dxcompiler.dll"
  $<TARGET_FILE_DIR:${BIN}>)