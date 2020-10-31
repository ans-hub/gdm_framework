// *************************************************************
// File:    helpers.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "helpers.h"

void gdm::scene::UpdateCamera(CameraEul& cam, MainInput& input, float dt)
{
  cam.Rotate(input.GetMouseY(), input.GetMouseX());

  dt += dt * static_cast<int>(input.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

  if (input.IsKeyboardBtnHold(DIK_W))
    cam.Move(-cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_S))
    cam.Move(cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_A))
    cam.Move(-cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_D))
    cam.Move(cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_R))
    cam.Move(cam.GetTm().GetCol(1), dt);
  if (input.IsKeyboardBtnHold(DIK_F))
    cam.Move(-cam.GetTm().GetCol(1), dt);
}
