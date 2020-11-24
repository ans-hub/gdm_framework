// *************************************************************
// File:    debug_widget.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "debug_widget.h"

#include <3rdparty/imgui/imgui.h>

// --public

namespace gdm::_private {

  bool InitImgui()
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    return true;
  }

} // namespace gdm::_private

gdm::DebugWidget::DebugWidget()
{
  static bool inited = _private::InitImgui();
}
