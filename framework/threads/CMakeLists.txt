
cmake_minimum_required (VERSION 3.10)

# --

project("gdm/framework/threads")
set(BIN threads)

message("* Lib: ${BIN} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(INCLUDE_DIRS
  "."
  "../../framework/"
)
include_directories(${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

# --

if (WIN32)
  set(OS "win")
else()
  set(OS "linux")
endif()

set(SRC_FILES
    ${OS}/critical_section.cc
    job_queue.cc
    job_manager.cc
    spin_lock.cc
    semaphore.cc
    thread.cc)

# --

add_library(${BIN} STATIC ${SRC_FILES})

