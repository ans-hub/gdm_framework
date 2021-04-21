// *************************************************************
// File:    playground_input_.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "app_input.h"

//--public

gdm::AppInput::AppInput(HWND window_handle, HINSTANCE hinstance)
  : api_input_{window_handle, hinstance}
  , text_button_{}
  , wire_button_{}
  , gui_buttons_{}
{}

void gdm::AppInput::UpdateCamera(CameraEul& cam, float dt)
{
  UpdateCameraRotate(cam, dt);
  UpdateCameraMovement(cam, dt);
}

void gdm::AppInput::UpdateCameraRotate(CameraEul& cam, float dt)
{
  cam.Rotate(api_input_.GetMouseY(), api_input_.GetMouseX());
}

void gdm::AppInput::UpdateCameraMovement(CameraEul& cam, float dt)
{
  dt += dt * static_cast<int>(api_input_.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

  if (api_input_.IsKeyboardBtnHold(DIK_W))
    cam.Move(cam.GetTm().GetCol(2), dt);
  if (api_input_.IsKeyboardBtnHold(DIK_S))
    cam.Move(-cam.GetTm().GetCol(2), dt);
  if (api_input_.IsKeyboardBtnHold(DIK_A))
    cam.Move(-cam.GetTm().GetCol(0), dt);
  if (api_input_.IsKeyboardBtnHold(DIK_D))
    cam.Move(cam.GetTm().GetCol(0), dt);
  if (api_input_.IsKeyboardBtnHold(DIK_R))
    cam.Move(cam.GetTm().GetCol(1), dt);
  if (api_input_.IsKeyboardBtnHold(DIK_F))
    cam.Move(-cam.GetTm().GetCol(1), dt);
}

void gdm::AppInput::UpdateLamps(CameraEul& cam, std::vector<ModelLight>& lamps, float dt)
{
  if (api_input_.IsKeyboardBtnPressed(DIK_1) && lamps.size() > 0)
    lamps[0].enabled_ = !lamps[0].enabled_;
  if (api_input_.IsKeyboardBtnPressed(DIK_2) && lamps.size() > 1)
    lamps[1].enabled_ = !lamps[1].enabled_;
  if (api_input_.IsKeyboardBtnPressed(DIK_3) && lamps.size() > 2)
    lamps[2].enabled_ = !lamps[2].enabled_;
  if (api_input_.IsKeyboardBtnPressed(DIK_4) && lamps.size() > 3)
    lamps[3].enabled_ = !lamps[3].enabled_;
}

void gdm::AppInput::UpdateFlashlights(CameraEul& cam, std::vector<ModelLight>& flashlights, float dt)
{
  if (api_input_.IsKeyboardBtnPressed(DIK_0) && flashlights.size() > 0)
    flashlights[0].enabled_ = !flashlights[0].enabled_;

  if (flashlights.size() > 0)
    flashlights[0].instance_.tm_ = cam.GetTm();
}

void gdm::AppInput::UpdateDebugDraw(DebugDraw& debug_draw)
{
  if (api_input_.IsKeyboardBtnPressed(wire_button_))
    debug_draw.ToggleActivateWire();
  if (api_input_.IsKeyboardBtnPressed(text_button_))
    debug_draw.ToggleActivateText();
}

void gdm::AppInput::UpdateGui(GuiManager& gui)
{
  for (auto&& [win, btn] : gui_buttons_)
    if (api_input_.IsKeyboardBtnPressed(btn))
      gui.ToggleActivateWindow(win);
}
