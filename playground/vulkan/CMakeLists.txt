cmake_minimum_required (VERSION 3.12)

project("gdm/framework/vk/vulkan")

set(BIN vulkan)
set(GDM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include(${GDM_ROOT_DIR}/tools/cmake_includes/framework.cmake)
include(${GDM_ROOT_DIR}/tools/cmake_includes/vulkan.cmake)
include(${GDM_ROOT_DIR}/tools/cmake_includes/dxc.cmake)
include(${GDM_ROOT_DIR}/tools/cmake_includes/freetype.cmake)

message("* App ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

add_definitions(-DGDM_LABELS_ENABLED) # ANS: please remove it after solve issues with microprofiler
add_definitions(-DGFX_VK_API) # ANS: looks like hack too

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(VK_API 1)
set(CMAKE_CXX_STANDARD 20)
set(PROFILING_ENABLED 0)

if ("${PROFILING_ENABLED}")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /O2")
  endif()
else()
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc") # todo: W4 ???
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()
endif()

set(SRC main.cc
        passes/gbuffer_pass.cc
        passes/deferred_pass.cc
        passes/debug_pass.cc
        passes/text_pass.cc
        scene/gpu_streamer.cc
        scene/scene_renderer.cc
        scene/data_helpers.cc
        scene/input_helpers.cc
        scene/scene.cc
        scene/debug_draw.cc
        scene/cfg_dispatcher.cc)

if (WIN32)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} /DWIN32 /D_WINDOWS /WX /wd4355 /wd4267"
      CACHE STRING "Debug compiler flags" FORCE)
  message("* App ${BIN}: set linker specific for Windows")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
endif()

message("* App ${BIN}: adding subdirectories")
add_subdirectory(${GDM_FRAMEWORK_DIR}/math/ static_libs/math)
add_subdirectory(${GDM_FRAMEWORK_DIR}/data/ static_libs/data)
add_subdirectory(${GDM_FRAMEWORK_DIR}/window/ static_libs/window)
add_subdirectory(${GDM_FRAMEWORK_DIR}/render/ static_libs/render)
add_subdirectory(${GDM_FRAMEWORK_DIR}/threads/ static_libs/threads)

message("* App ${BIN}: adding executable")
add_executable(${BIN} ${SRC})

message("* App ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ${CMAKE_THREAD_LIBS_INIT} math system data window render threads)
target_link_libraries(${BIN} winmm dxgi dinput8 dxguid)
if(WIN32)
  target_link_libraries(${BIN} wsock32 ws2_32)
endif()

include(${GDM_ROOT_DIR}/tools/cmake_includes/vulkan_post.cmake)
include(${GDM_ROOT_DIR}/tools/cmake_includes/dxc_post.cmake)
include(${GDM_ROOT_DIR}/tools/cmake_includes/freetype_post.cmake)
