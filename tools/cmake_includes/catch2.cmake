message("* App ${BIN}: adding catch2 package")

set(GDM_LIBCATCH_DIR ${GDM_ROOT_DIR}/3rdparty/catch)

include_directories(
  ${GDM_LIBCATCH_DIR}
)
