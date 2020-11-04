// *************************************************************
// File:    scene_manager.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_SCENE_MGR
#define GFX_VK_SCENE_MGR

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

struct RenderableMaterials;

struct SceneManager
{
  SceneManager(api::Device& device);

  void CreateDummyView(api::CommandList& cmd);
  uint CreateStagingBuffer(uint bytes);
  auto GetStagingBuffer(uint index) -> api::Buffer& { return *staging_buffers_[index]; };
  auto LoadAbstractModels(const Config& cfg) -> std::vector<ModelHandle>;
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, uint vstg_index, uint istg_index, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<ModelHandle>& models, uint tstg_index, api::CommandList& list);
  void UpdateCamera(CameraEul& cam, MainInput& input, float dt);
  auto GetRenderableModels() -> const std::set<ModelHandle>& { return models_; }
  auto GetRenderableMaterials() -> RenderableMaterials;

public:
  constexpr static uint v_max_materials = 16;

private:
  auto GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>;
  uint CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset);

private:
  api::Device& device_;  
  std::set<ModelHandle> models_;
  std::vector<api::Buffer*> staging_buffers_;
  api::ImageView* dummy_view_;

private:
  static constexpr const char* v_model_prefix = "model_";
  static constexpr const char* v_model_pos_prefix = "model_pos_";

};  // struct SceneManager

struct RenderableMaterials
{
  api::ImageViews diffuse_views_;
  api::ImageViews specular_views_;
  api::ImageViews normal_views_;

};  // struct RenderableMaterials

} // namespace gdm::scene

#endif // GFX_VK_SCENE_MGR