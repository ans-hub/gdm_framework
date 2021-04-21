// *************************************************************
// File:    app_helpers.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_VK_SCENE_DATA_HELPERS_H
#define GDM_VK_SCENE_DATA_HELPERS_H

#include <set>
#include <unordered_map>

#include "engine/camera_eul.h"
#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"

#include "memory/defines.h"

#include "window/main_input.h"

#include "factory/model_factory.h"
#include "factory/material_factory.h"
#include "factory/texture_factory.h"
#include "factory/image_factory.h"
#include "data/cfg_loader.h"

namespace gdm::app_helpers {

  auto LoadObjects(const Config& cfg) -> std::vector<ModelInstance>;
  auto LoadObjectNames(const Config& cfg) -> std::vector<std::string>;
  auto LoadLights(const Config& cfg) -> std::vector<ModelInstance>;
  auto LoadFlashlights(const Config& cfg) -> std::vector<ModelInstance>;
  auto GetUniqueModels(const std::vector<ModelInstance*>& instances) -> std::vector<ModelHandle>;

} // namespace gdm::helpers

#endif // GDM_VK_SCENE_DATA_HELPERS_H