// *************************************************************
// File:    gui_manager.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gui_manager.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-- public

gdm::GuiManager::GuiManager()
  : callbacks_{}
{ }

auto gdm::GuiManager::RegisterGuiCallback(GuiCallback::Fn&& fn) -> int
{
  callbacks_.emplace_back(fn, false);

  return static_cast<int>(callbacks_.size() - 1);
}

bool gdm::GuiManager::IsActiveAnyWindow() const
{
  bool is_active = false;
  for (auto&& [cb, active] : callbacks_)
    is_active |= active;  
  return is_active;
}

//-- helpers

bool gdm::gui::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui::GetCurrentContext() == NULL)
      return false;

  ImGuiIO& io = ImGui::GetIO();
  
  if (io.WantCaptureKeyboard || io.WantCaptureMouse) 
  {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
    return true;
  }
  return false;
};
