// *************************************************************
// File:    cfg_dispatcher.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "cfg_dispatcher.h"

#include <string>

#include "system/diff_utils.h"

#include "logic/tennis.cfg.cc"
#include "logic/spatial.cfg.cc"

//--private

namespace gdm::_private
{
  static void UpdateDummy(cfg::Models&, CameraEul&, MainInput&, DebugDraw&, float) { }
}

//--public

gdm::cfg::Dispatcher::Dispatcher()
  : logic_{gdm::_private::UpdateDummy}
{ }

gdm::cfg::Dispatcher::Dispatcher(Config& cfg, std::vector<ModelInstance*> models, const std::vector<std::string>& names)
  : logic_{gdm::_private::UpdateDummy}
{
  for (auto&& [model, name] : range::ZipSpan(models, names))
    models_[name] = model;

  if (cfg.GetSname() == std::string("tennis.cfg"))
    logic_ = &gdm::_private::UpdateTennis;
  else if (cfg.GetSname() == std::string("spatial.cfg"))
    logic_ = &gdm::_private::UpdateSpatial;
}

void gdm::cfg::Dispatcher::Update(CameraEul& cam, MainInput& input, DebugDraw& debug, float dt)
{
  return logic_(models_, cam, input, debug, dt);
}
