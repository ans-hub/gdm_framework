// *************************************************************
// File:    scene.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_SCENE_HELPERS
#define GFX_VK_SCENE_HELPERS

#include <set>

#include "render/camera_eul.h"
#include "render/api.h"
#include "render/defines.h"
#include "render/renderer.h"

#include "memory/defines.h"

#include "window/main_input.h"

#include "data/model_factory.h"
#include "data/material_factory.h"
#include "data/texture_factory.h"
#include "data/image_factory.h"

namespace gdm {

struct Scene
{
  Scene(api::Device& device);

  auto LoadAbstractModels(const Config& cfg) -> std::vector<ModelHandle>;
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, api::Buffer& vstg, api::Buffer& istg, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<ModelHandle>& models, api::Buffer& tstg, api::CommandList& list);
  void UpdateCamera(CameraEul& cam, MainInput& input, float dt);
  auto GetModels() -> const std::set<ModelHandle>& { return models_; }

private:
  api::Device& device_;  
  std::set<ModelHandle> models_;

private:
  static constexpr const char* v_model_prefix = "model_";
  static constexpr const char* v_model_pos_prefix = "model_pos_";

};  // struct Scene

} // namespace gdm::scene

#endif // GFX_VK_SCENE_HELPERS