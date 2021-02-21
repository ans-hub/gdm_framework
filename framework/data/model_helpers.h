// *************************************************************
// File:    model_helpers.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MODEL_HELPERS_H
#define AH_GDM_MODEL_HELPERS_H

#include <vector>
#include <array>
#include <type_traits>

#include "cfg_loader.h"
#include "model_factory.h"

namespace gdm {
  struct ModelInstance;
}

namespace gdm::helpers {

  auto LoadObjects(const Config& cfg) -> std::vector<ModelInstance>;
  auto LoadObjectNames(const Config& cfg) -> std::vector<std::string>;
  auto LoadLights(const Config& cfg) -> std::vector<ModelInstance>;
  auto LoadFlashlights(const Config& cfg) -> std::vector<ModelInstance>;
  auto GetUniqueModels(const std::vector<ModelInstance>& objs, const std::vector<ModelInstance>& lamps) -> std::vector<ModelHandle>;

} // namespace gdm::helpers

#endif // AH_GDM_MODEL_HELPERS_H