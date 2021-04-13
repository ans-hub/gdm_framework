// *************************************************************
// File:    app_gui.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_PLAYGROUND_GUI_H
#define GDM_PLAYGROUND_GUI_H

#include <vector>
#include <functional>

#include "Windows.h"

#include "imgui.h"

namespace gdm::gui {

  void GuiExampleCb();
  void GuiDockingCb();

} // namespace gdm::gui

#endif // GDM_PLAYGROUND_GUI_H
