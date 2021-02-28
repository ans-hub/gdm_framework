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

namespace gdm::helpers {

  void UpdateCamera(CameraEul& cam, MainInput& input, float dt);
  void UpdateLamps(CameraEul& cam, MainInput& input, std::vector<ModelLight>& lamps, float dt);
  void UpdateFlashlights(CameraEul& cam, MainInput& input, std::vector<ModelLight>& flashlights, float dt);

} // namespace gdm::helpers

#endif // GDM_VK_SCENE_INPUT_HELPERS_H