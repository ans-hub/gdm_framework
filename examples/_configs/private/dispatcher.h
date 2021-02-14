// *************************************************************
// File:    dispatcher.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_CONFIG_DISPATCHER_H
#define GFX_VK_CONFIG_DISPATCHER_H

#include <functional>

#include "render/camera_eul.h"
#include "window/main_input.h"
#include "data/model_factory.h"
#include "data/cfg_loader.h"

namespace gdm::cfg {

struct Dispatcher
{
  Dispatcher(Config& cfg, std::vector<ModelInstance*> models, const std::vector<std::string>& names);
  void Update(CameraEul& cam, MainInput& input, float dt);

private:
  Config& cfg_;
  std::function<void(std::unordered_map<std::string, ModelInstance*>&, CameraEul&, MainInput&, float)> logic_;  
  std::unordered_map<std::string, ModelInstance*> models_;
};

} // namespace gdm::cfg

#endif // GFX_VK_CONFIG_DISPATCHER_H