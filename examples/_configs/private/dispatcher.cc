// *************************************************************
// File:    config_dispatcher.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dispatcher.h"

#include <string>

#include "system/diff_utils.h"

#include "../tennis.cfg.cc"
#include "../wire.cfg.cc"

//--private

namespace gdm::_private
{
  static void UpdateDummy(cfg::Models&, CameraEul&, MainInput&, DebugDraw&, float) { }
}

//--public

gdm::cfg::Dispatcher::Dispatcher(Config& cfg, std::vector<ModelInstance*> models, const std::vector<std::string>& names)
  : cfg_{cfg}
  , logic_{gdm::_private::UpdateDummy}
  , debug_on_{false}
{
  for (auto&& [model, name] : range::ZipSpan(models, names))
    models_[name] = model;

  if (cfg.GetSname() == std::string("tennis.cfg"))
    logic_ = &gdm::_private::UpdateTennis;
}

void gdm::cfg::Dispatcher::Update(CameraEul& cam, MainInput& input, DebugDraw& debug, float dt)
{
  debug_on_ ^= input.IsKeyboardBtnPressed(DIK_TAB);

  return logic_(models_, cam, input, debug, dt);
}
