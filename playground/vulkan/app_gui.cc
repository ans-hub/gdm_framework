// *************************************************************
// File:    app_gui.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "app_gui.h"

#include "imgui_impl_win32.h"

#if defined (GFX_VK_API)
#include "imgui_impl_vulkan.h"
#else
#include "imgui_impl_dx11.h"
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

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

void gdm::gui::GuiDockingCb()
{
  ImGui::ShowUserGuide();
}
