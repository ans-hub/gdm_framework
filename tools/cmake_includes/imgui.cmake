message("* App ${BIN}: adding imgui")

set(GDM_LIBIMGUI_DIR ${GDM_ROOT_DIR}/3rdparty/imgui)

include_directories(
  ${GDM_LIBIMGUI_DIR}
)

set(SRC ${SRC}
  ${GDM_LIBIMGUI_DIR}/imgui.cpp
  ${GDM_LIBIMGUI_DIR}/examples/imgui_impl_vulkan.cpp
  ${GDM_LIBIMGUI_DIR}/examples/imgui_impl_win32.cpp
)
