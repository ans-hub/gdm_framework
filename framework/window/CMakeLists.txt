cmake_minimum_required (VERSION 3.10)

# --

project("gdm/framework/window")

set(BIN window)
set(GDM_LIBIMGUI_DIR ${GDM_ROOT_DIR}/3rdparty/imgui)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(CMAKE_CXX_STANDARD 20)

# --

include_directories(
  "."
  ${GDM_LIBIMGUI_DIR}/
  ${GDM_FRAMEWORK_DIR}/window/
  ${GDM_FRAMEWORK_DIR}/window/linux/
  ${GDM_FRAMEWORK_DIR}/window/win/
  ${INCLUDE_DIRS}
)

# --

set(SRC_FILES
  ${GDM_LIBIMGUI_DIR}/imgui.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_demo.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_draw.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_widgets.cpp)

if (UNIX)
  set(SRC_FILES
    ${SRC_FILES}
    linux/x_window.cc
    linux/main_window.cc
    linux/helpers.cc)
else()
  set(SRC_FILES
    ${SRC_FILES}
    win/dx_input.cc
    win/main_window.cc)
endif()

# --

add_library(${BIN} STATIC ${SRC_FILES})

# requies target_link_libraries(${BIN} winmm dinput8 dxguid) in target app