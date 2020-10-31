
cmake_minimum_required (VERSION 3.10)

# --

project("gdm/framework/system")
set(BIN system)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(INCLUDE_DIRS
  "."
  "../../"  # root
)
include_directories(${INCLUDE_DIRS})

# --

set(SRC_FILES
    fps_counter.cc
    string_utils.cc
    timer.cc
    mesh_utils.cc
    logger.cc)

if ("${PROFILING_ENABLED}")
  message("  ** using microprofiler")
  set(SRC_FILES "${SRC_FILES};../../3rdparty/microprofile/microprofile.cpp")
  add_definitions(-DMICROPROFILE_GPU_TIMERS=0
                  -DMICROPROFILE_WEBSERVER=1
                  -DGDM_PROFILER_ENABLED=1
                  -DGDM_LOGGER_ENABLED=1
                  -DMICROPROFILE_LOCALE_INDEPENDENT_FLOAT=1)
else()
  message("  ** no microprofiler")
endif()

# --

add_library(${BIN} STATIC ${SRC_FILES})
