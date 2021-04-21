// *************************************************************
// File:    app_input.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_VK_PLAYGROUND_INPUT_H
#define GDM_VK_PLAYGROUND_INPUT_H

#include <map>
#include <vector>

#include "engine/camera_eul.h"
#include "window/main_input.h"
#include "factory/model_factory.h"

#include "engine/debug_draw.h"
#include "engine/gui_manager.h"

namespace gdm {

struct AppInput
{
  AppInput(HWND window_handle, HINSTANCE hinstance);

  void SetGuiButton(int window_id, uint button) { gui_buttons_[window_id] = button; }
  void SetTextButton(uint button) { text_button_ = button; }
  void SetWireButton(uint button) { wire_button_ = button; }

  auto GetRawInput() const -> const MainInput& { return api_input_; }
  auto GetRawInput() -> MainInput& { return api_input_; }

  void Update() { }
  void UpdateCamera(CameraEul& cam, float dt);
  void UpdateCameraRotate(CameraEul& cam, float dt);
  void UpdateCameraMovement(CameraEul& cam, float dt);
  void UpdateLamps(CameraEul& cam, std::vector<ModelLight>& lamps, float dt);
  void UpdateFlashlights(CameraEul& cam, std::vector<ModelLight>& flashlights, float dt);
  void UpdateDebugDraw(DebugDraw& debug_draw);
  void UpdateGui(GuiManager& gui);

private:
  MainInput api_input_;
  unsigned char text_button_;
  unsigned char wire_button_;
  std::map<int, unsigned char> gui_buttons_;

}; // struct AppInput

} // namespace gdm

#endif // GDM_VK_PLAYGROUND_INPUT_H
