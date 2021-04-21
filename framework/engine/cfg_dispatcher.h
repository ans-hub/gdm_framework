// *************************************************************
// File:    cfg_dispatcher.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_CONFIG_DISPATCHER_H
#define GFX_CONFIG_DISPATCHER_H

#include <functional>

#include "engine/camera_eul.h"
#include "window/main_input.h"
#include "factory/model_factory.h"
#include "data/cfg_loader.h"
#include "engine/debug_draw.h"

#include "defines.h"

namespace gdm::cfg {

struct Dispatcher
{
  Dispatcher();
  Dispatcher(Config& cfg, std::vector<ModelInstance*> models, const std::vector<std::string>& names);

  void Update(CameraEul& cam, const MainInput& input, DebugDraw& debug, float dt);

private:
  Callback logic_;  
  Models models_;

}; // struct Dispatcher

} // namespace gdm::cfg

#endif // GFX_CONFIG_DISPATCHER_H