
cmake_minimum_required (VERSION 3.10)

# --

project("gdm/framework/system")

set(BIN system)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

include_directories(
    "system/"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIRS}
)

# --

set(SRC_FILES
    fps_counter.cc
    string_utils.cc
    timer.cc
    mesh_utils.cc
    debugging.cc
    type_id.cc
    logger.cc)

if ("${PROFILING_ENABLED}")
  message("  ** using microprofiler")
  set(SRC_FILES "${SRC_FILES};../../3rdparty/microprofile/microprofile.cpp")
else()
  message("  ** no microprofiler")
endif()

# --

add_library(${BIN} STATIC ${SRC_FILES})

if ("${PROFILING_ENABLED}")
  set(PROFILING_DEFINITIONS
    -DMICROPROFILE_WEBSERVER=1
    -DGDM_PROFILER_ENABLED=1
    -DGDM_LOGGER_ENABLED=1)

  if (NOT "${DISABLE_GPU_PROFILING}")
    set(PROFILING_DEFINITIONS
      ${PROFILING_DEFINITIONS}
      -DMICROPROFILE_GPU_TIMERS=1
      -DMICROPROFILE_LOCALE_INDEPENDENT_FLOAT=1
      -DMICROPROFILE_GPU_TIMERS_VULKAN=1)
  else()
    set(PROFILING_DEFINITIONS
      ${PROFILING_DEFINITIONS}
      -DMICROPROFILE_GPU_TIMERS=0)
  endif()
  
  target_compile_definitions(${BIN} PUBLIC ${PROFILING_DEFINITIONS})

#  if (${VK_API} EQUAL "1")
#  target_compile_definitions(${BIN} PUBLIC
#  endif()
endif()
