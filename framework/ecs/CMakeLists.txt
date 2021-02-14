cmake_minimum_required (VERSION 3.10)

# -- Project initials --

project("gdm/framework/ecs/")

set(BIN ecs)
set(GDM_LIBSYSTEM_DIR ${GDM_ROOT_DIR}/framework/system)

message("* Lib ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
message("  ** into directory: ${CMAKE_BINARY_DIR}")

# --

set(CMAKE_CXX_STANDARD 20)

# --

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-ansi -pedantic -Wall -Wextra -Werror")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /Zc:preprocessor /Wv:18 /wd4355 /wd4267")
endif()

# -- Include

include_directories(
    "."
    ${INCLUDE_DIRS}
)

# -- Sources

set(SRC_FILES
  manager.cc
  system.cc)

# -- Libs

message("* Lib ${BIN}: adding libs")
add_subdirectory(${GDM_LIBSYSTEM_DIR} gdm_libs/system)

# -- Executable --

message("* Lib ${BIN}: adding executable")
add_library(${BIN} STATIC ${SRC_FILES})

# -- Link --

message("* Lib ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ${CMAKE_THREAD_LIBS_INIT} system)