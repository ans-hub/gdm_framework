cmake_minimum_required (VERSION 3.10)

project("gdm/framework/memory/")

set(BIN memory)

message("* Lib ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(CMAKE_CXX_STANDARD 17)

# --

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# -- Include

include_directories(
    "data/"
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${INCLUDE_DIRS}
)

# -- Sources

set(SRC_FILES
  helpers.cc
  operators.cc
  memory_tracker.cc
  memory_manager.cc)

# -- Libs

message("* Lib ${BIN}: adding libs")
add_subdirectory(${GDM_FRAMEWORK_DIR}/system gdm_libs/system)

# -- Executable --

message("* Lib ${BIN}: adding executable")
add_library(${BIN} STATIC ${SRC_FILES})

# -- Link --

message("* Lib ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ${CMAKE_THREAD_LIBS_INIT} system)