// *************************************************************
// File:    config_dispatcher.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dispatcher.h"

#include <string>

#include "system/diff_utils.h"

#include "../tennis.cfg.cc"

//--private

namespace gdm::_private
{
  static void UpdateDummy(std::unordered_map<std::string, ModelInstance*>&, CameraEul&, MainInput&, float) { }
}

//--public

gdm::cfg::Dispatcher::Dispatcher(Config& cfg, std::vector<ModelInstance*> models, const std::vector<std::string>& names)
  : cfg_{cfg}
  , logic_{gdm::_private::UpdateDummy}
{
  for (auto&& [model, name] : range::ZipSpan(models, names))
    models_[name] = model;

  std::string s1 = cfg.GetSname();
  std::string s2 = std::string("tennis.cfg");

  if (cfg.GetSname() == std::string("tennis.cfg"))
    logic_ = &gdm::_private::UpdateTennis;
}

void gdm::cfg::Dispatcher::Update(CameraEul& cam, MainInput& input, float dt)
{
  logic_(models_, cam, input, dt);
}
