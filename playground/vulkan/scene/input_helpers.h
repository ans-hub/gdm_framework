// *************************************************************
// File:    input_helpers.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_VK_SCENE_INPUT_HELPERS_H
#define GDM_VK_SCENE_INPUT_HELPERS_H

#include "render/camera_eul.h"

#include "window/main_input.h"

#include "data/model_factory.h"

#include "scene/debug_draw.h"
#include "scene/gui_draw.h"

namespace gdm::input_helpers {

  void UpdateCamera(CameraEul& cam, MainInput& input, float dt);
  void UpdateCameraRotate(CameraEul& cam, MainInput& input, float dt);
  void UpdateCameraMovement(CameraEul& cam, MainInput& input, float dt);
  void UpdateLamps(CameraEul& cam, MainInput& input, std::vector<ModelLight>& lamps, float dt);
  void UpdateFlashlights(CameraEul& cam, MainInput& input, std::vector<ModelLight>& flashlights, float dt);
  void UpdateDebugDraw(MainInput& input, DebugDraw& debug_draw);
  void UpdateGuiDraw(MainInput& input, GuiDraw& gui);

} // namespace gdm::helpers

#endif // GDM_VK_SCENE_INPUT_HELPERS_H