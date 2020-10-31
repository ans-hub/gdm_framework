// *************************************************************
// File:    helpers.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_SCENE_HELPERS
#define GFX_VK_SCENE_HELPERS

#include "render/camera_eul.h"
#include "render/api.h"
#include "window/main_input.h"

namespace gdm::scene {

  void UpdateCamera(CameraEul& cam, MainInput& input, float dt);

}

#endif // GFX_VK_SCENE_HELPERS