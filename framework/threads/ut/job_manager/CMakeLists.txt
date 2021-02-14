cmake_minimum_required (VERSION 3.10)

# --

project("gdm/framework/job_manager: test")
set(BIN job_manager_test)

message("* App ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(PROFILING_ENABLED 1)

# --

if ("${PROFILING_ENABLED}")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /O2")
  endif()
else()
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
      set(warnings "/W4 /WX /EHsc")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()
endif()

# --

set(INCLUDE_DIRS
  "."
  "../"
  "../../"
  "../../../../framework/"
  "../../../../"
)
include_directories(${INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

# --

find_package(Threads)

message("* App ${BIN}: adding subdirectories")
add_subdirectory(../../../../framework/profile/ static_libs/system)
add_subdirectory(../../../../framework/threads/ static_libs/threads)

message("* App ${BIN}: adding executable")
add_executable(${BIN} ${SRC_FILES} job_manager_test.cc)

# --

message("* App ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ${CMAKE_THREAD_LIBS_INIT} profile system)
if(WIN32)
  target_link_libraries(${BIN} wsock32 ws2_32)
endif()
