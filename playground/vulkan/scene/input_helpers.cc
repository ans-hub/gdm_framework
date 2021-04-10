// *************************************************************
// File:    input_helpers.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "input_helpers.h"

//--public

void gdm::input_helpers::UpdateCamera(CameraEul& cam, MainInput& input, float dt)
{
  UpdateCameraRotate(cam, input, dt);
  UpdateCameraMovement(cam, input, dt);
}

void gdm::input_helpers::UpdateCameraRotate(CameraEul& cam, MainInput& input, float dt)
{
  cam.Rotate(input.GetMouseY(), input.GetMouseX());
}

void gdm::input_helpers::UpdateCameraMovement(CameraEul& cam, MainInput& input, float dt)
{
  dt += dt * static_cast<int>(input.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

  if (input.IsKeyboardBtnHold(DIK_W))
    cam.Move(cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_S))
    cam.Move(-cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_A))
    cam.Move(-cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_D))
    cam.Move(cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_R))
    cam.Move(cam.GetTm().GetCol(1), dt);
  if (input.IsKeyboardBtnHold(DIK_F))
    cam.Move(-cam.GetTm().GetCol(1), dt);
}

void gdm::input_helpers::UpdateLamps(CameraEul& cam, MainInput& input, std::vector<ModelLight>& lamps, float dt)
{
  if (input.IsKeyboardBtnPressed(DIK_1) && lamps.size() > 0)
    lamps[0].enabled_ = !lamps[0].enabled_;
  if (input.IsKeyboardBtnPressed(DIK_2) && lamps.size() > 1)
    lamps[1].enabled_ = !lamps[1].enabled_;
  if (input.IsKeyboardBtnPressed(DIK_3) && lamps.size() > 2)
    lamps[2].enabled_ = !lamps[2].enabled_;
  if (input.IsKeyboardBtnPressed(DIK_4) && lamps.size() > 3)
    lamps[3].enabled_ = !lamps[3].enabled_;
}

void gdm::input_helpers::UpdateFlashlights(CameraEul& cam, MainInput& input, std::vector<ModelLight>& flashlights, float dt)
{
  if (input.IsKeyboardBtnPressed(DIK_0) && flashlights.size() > 0)
    flashlights[0].enabled_ = !flashlights[0].enabled_;

  if (flashlights.size() > 0)
    flashlights[0].instance_.tm_ = cam.GetTm();
}

void gdm::input_helpers::UpdateDebugDraw(MainInput& input, DebugDraw& debug_draw)
{
  if (input.IsKeyboardBtnPressed(DIK_F11))
    debug_draw.ToggleActive();
}

void gdm::input_helpers::UpdateGuiDraw(MainInput& input, GuiDraw& gui_draw)
{
  if (input.IsKeyboardBtnPressed(DIK_F9))
    gui_draw.ToggleActive(GuiDraw::EStage::BG_TEXT);

  if (input.IsKeyboardBtnPressed(DIK_F10))
    gui_draw.ToggleActive(GuiDraw::EStage::WINDOWS);
}
