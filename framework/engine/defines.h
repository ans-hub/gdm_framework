// *************************************************************
// File:    defines.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_ENGINE_DEFINES_H
#define GFX_ENGINE_DEFINES_H

#include <unordered_map>
#include <functional>

#include <data/model_factory.h>
#include <engine/camera_eul.h>
#include <render/defines.h>
#include <engine/debug_draw.h>
#include <window/main_input.h>

namespace gdm::cfg {

  using Dt = float;
  using Models = std::unordered_map<std::string, ModelInstance*>;
  using Callback = std::function<void(Models&, CameraEul&, const MainInput&, DebugDraw&, Dt)>;

  constexpr static const char* v_dummy_image = "dummy_handle";

} // namespace gdm::cfg

#endif // GFX_ENGINE_DEFINES_H