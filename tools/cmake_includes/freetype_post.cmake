message("* App ${BIN}: post action for freetype")

target_link_options(${BIN} PUBLIC "/ignore:4099")
target_link_libraries(${BIN} ${FREETYPE_LIBRARY})
