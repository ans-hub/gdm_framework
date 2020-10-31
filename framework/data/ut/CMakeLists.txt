cmake_minimum_required (VERSION 3.10)

# -- Project initials --

project("gdm/framework/data/ut")

set(BIN data_ut)
set(GDM_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(GDM_FRAMEWORK_DIR ${GDM_ROOT_DIR}/framework)
set(GDM_LIBCATCH_DIR ${GDM_ROOT_DIR}/3rdparty/catch)

message("* App ${BIN}: ${CMAKE_PROJECT_NAME} (${CMAKE_BUILD_TYPE})")
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
    ${GDM_ROOT_DIR}
    ${GDM_FRAMEWORK_DIR}
    ${GDM_LIBCATCH_DIR}
    ${INCLUDE_DIRS}
)

# -- Sources

set(SRC_FILES
  data_ut.cc)

# -- Libs

message("* App ${BIN}: adding libs")
add_subdirectory(${GDM_FRAMEWORK_DIR}/data/ gdm_libs/data)
add_subdirectory(${GDM_LIBCATCH_DIR} gdm_libs/catch)

# -- Executable --

message("* App ${BIN}: adding executable")
add_executable(${BIN} ${SRC_FILES})

# -- Link --

message("* App ${BIN}: linking 3rd libraries")
target_link_libraries(${BIN} ${CMAKE_THREAD_LIBS_INIT} data catch)