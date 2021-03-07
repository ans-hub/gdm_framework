message("* App ${BIN}: adding dxc package")

set(GDM_LIBDXC_DIR ${GDM_ROOT_DIR}/3rdparty/dxc)

include_directories(
  ${GDM_LIBDXC_DIR}/include
)
