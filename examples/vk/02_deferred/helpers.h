// *************************************************************
// File:    helpers.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_SCENE_HELPERS
#define GFX_VK_SCENE_HELPERS

#include <set>

#include "render/camera_eul.h"
#include "render/api.h"
#include "render/defines.h"
#include "render/renderer.h"

#include "memory/defines.h"

#include "window/main_input.h"

#include "data/model_factory.h"
#include "data/material_factory.h"
#include "data/texture_factory.h"
#include "data/image_factory.h"

namespace gdm::helpers {

auto LoadAbstractModels(const Config& cfg) -> std::vector<ModelHandle>;
void UpdateCamera(CameraEul& cam, MainInput& input, float dt);

} // namespace gdm::helpers

#endif // GFX_VK_SCENE_HELPERS