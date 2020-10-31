cmake_minimum_required (VERSION 3.10)

# --

project("gdm/framework/window")
set(BIN window)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(CMAKE_CXX_STANDARD 17)

# --

include_directories(
  "."
  "linux"
  "win"
  "../../framework/"
  "../../framework/window/"
  ${INCLUDE_DIRS}
)

# --

if (UNIX)
  set(SRC_FILES
      linux/x_window.cc
      linux/main_window.cc
      linux/helpers.cc)
else()
  set(SRC_FILES
      win/dx_input.cc
      win/main_window.cc)
endif()

# --

add_library(${BIN} STATIC ${SRC_FILES})

# requies target_link_libraries(${BIN} winmm dinput8 dxguid) in target app