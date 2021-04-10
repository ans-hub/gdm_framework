// *************************************************************
// File:    gui_draw.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gui_draw.h"

#include "3rdparty/imgui/examples/imgui_impl_win32.h"
#include "3rdparty/imgui/examples/imgui_impl_vulkan.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-- public

gdm::GuiDraw::GuiDraw()
  : is_active_{false}
{ }

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

void gdm::gui::GuiExampleCb()
{
  static float f = 0.0f;
  static int counter = 0;
  ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
  ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
  bool show;
  ImGui::Checkbox("Demo Window", &show);                  // Edit bools storing our window open/close state
  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
  if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
      counter++;
  ImGui::SameLine();
  ImGui::Text("counter = %d", counter);
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();
}
