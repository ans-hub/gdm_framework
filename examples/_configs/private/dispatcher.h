// *************************************************************
// File:    dispatcher.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_CONFIG_DISPATCHER_H
#define GFX_VK_CONFIG_DISPATCHER_H

#include <functional>

#include "types.h"

#include "render/camera_eul.h"
#include "render/debug_draw.h"
#include "window/main_input.h"
#include "data/model_factory.h"
#include "data/cfg_loader.h"

namespace gdm::cfg {

struct Dispatcher
{
  Dispatcher(Config& cfg, std::vector<ModelInstance*> models, const std::vector<std::string>& names);

  void Update(CameraEul& cam, MainInput& input, DebugDraw& debug, float dt);
  bool IsDebugMode() const { return debug_on_; }

private:
  Config& cfg_;
  Callback logic_;  
  Models models_;
  bool debug_on_;

}; // struct Dispatcher

} // namespace gdm::cfg

#endif // GFX_VK_CONFIG_DISPATCHER_H