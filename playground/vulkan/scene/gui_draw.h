// *************************************************************
// File:    gui_draw.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_GUI_DRAW_H
#define GDM_GUI_DRAW_H

#include "Windows.h"

#include "3rdparty/imgui/imgui.h"

namespace gdm {
  
struct GuiDraw
{
  enum class EStage
  {
    WINDOWS,
    BG_TEXT,
    Max
  }; // enum class EStage

  GuiDraw();

  bool IsActive(EStage s) const { return is_active_[static_cast<int>(s)]; }
  void ToggleActive(EStage s) { is_active_[static_cast<int>(s)] ^= true; }

private:
  bool is_active_[static_cast<int>(EStage::Max)];

}; // struct GuiDraw

} // namespace gdm

namespace gdm::gui {

  bool WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  void GuiExampleCb();

} // namespace gdm::gui

#endif // GDM_GUI_DRAW_H
