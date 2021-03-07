message("* App ${BIN}: post action for dxc")

add_custom_command(TARGET ${BIN} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${GDM_LIBDXC_DIR}/bin/dxcompiler.dll"
  $<TARGET_FILE_DIR:${BIN}>)