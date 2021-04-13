// *************************************************************
// File:    gui_manager.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_GUI_MANAGER_H
#define GDM_GUI_MANAGER_H

#include <vector>
#include <functional>

#include "Windows.h"

#include "imgui.h"

namespace gdm {

struct GuiCallback
{
  using Fn = std::function<void()>;
  Fn fn_;
  bool active_;

}; // struct GuiCallback

using GuiCallbacks = std::vector<GuiCallback>;

struct GuiManager
{
  GuiManager();

public:
  auto RegisterGuiCallback(GuiCallback::Fn&& fn) -> int;
  auto GetGuiCallbacks() const { return callbacks_; }

public:
  bool IsActiveWindow(int window_id) const { return callbacks_[window_id].active_; }
  bool IsActiveAnyWindow() const;

public:
  void ToggleActivateWindow(int window_id) { callbacks_[window_id].active_ ^= true; }
  void ActivateWindow(int window_id, bool activate) { callbacks_[window_id].active_ = activate; }

private:
  GuiCallbacks callbacks_;

}; // struct GuiManager

} // namespace gdm

namespace gdm::gui {

  bool WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

} // namespace gdm::gui

#endif // GDM_GUI_MANAGER_H
