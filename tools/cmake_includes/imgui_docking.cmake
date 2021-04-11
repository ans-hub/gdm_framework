message("* App ${BIN}: adding imgui")

set(GDM_LIBIMGUI_DIR ${GDM_ROOT_DIR}/3rdparty/imgui/docking/imgui)

include_directories(
  ${GDM_LIBIMGUI_DIR}
  ${GDM_LIBIMGUI_DIR}/backends
  ${GDM_LIBIMGUI_DIR}/..
)

set(SRC ${SRC}
  ${GDM_LIBIMGUI_DIR}/imgui.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_draw.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_widgets.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_tables.cpp
  ${GDM_LIBIMGUI_DIR}/imgui_demo.cpp
  ${GDM_LIBIMGUI_DIR}/backends/imgui_impl_vulkan.cpp
  ${GDM_LIBIMGUI_DIR}/backends/imgui_impl_win32.cpp
)
