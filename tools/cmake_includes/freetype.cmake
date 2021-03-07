message("* App ${BIN}: adding freetype package")

include(${GDM_ROOT_DIR}/tools/cmake_includes/framework.cmake)

set(GDM_LIBFREETYPE_DIR ${GDM_ROOT_DIR}/3rdparty/freetype/2.10.04)

include_directories(
  ${GDM_LIBFREETYPE_DIR}/include
)

set(FREETYPE_LIBRARY ${GDM_LIBFREETYPE_DIR}/release_static/win64/freetype.lib)
set(FREETYPE_INCLUDE_DIRS ${GDM_LIBFREETYPE_DIR}/include)
find_package(Freetype REQUIRED FATAL_ERROR)
