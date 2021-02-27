// *************************************************************
// File:    types.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_TYPES_DISPATCHER_H
#define GFX_TYPES_DISPATCHER_H

#include <functional>
#include <unordered_map>

#include "render/camera_eul.h"
#include "render/debug_draw.h"
#include "window/main_input.h"
#include "data/model_factory.h"
#include "data/cfg_loader.h"

namespace gdm::cfg {

using Models = std::unordered_map<std::string, ModelInstance*>;
using Dt = float;
using Callback = std::function<void(Models&, CameraEul&, MainInput&, DebugDraw&, Dt)>;

} // namespace gdm::cfg

#endif // GFX_TYPES_DISPATCHER_H