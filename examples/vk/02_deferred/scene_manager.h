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
  void SetModels(const std::vector<ModelHandle>& models);
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, uint vstg_index, uint istg_index, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<ModelHandle>& models, uint tstg_index, api::CommandList& list);
  auto GetRenderableModels() -> const std::set<ModelHandle>& { return models_; }
  auto GetRenderableMaterials() -> RenderableMaterials;
  auto GetPerFrameUBO(uint frame_num) -> api::Buffer* { return pfcb_uniform_[frame_num]; }
  auto GetPerObjectUBO(uint frame_num) -> api::Buffer* { return pocb_uniform_[frame_num]; }

  template<class T, uint Count>
  void CreatePerFrameUBO(api::CommandList& cmd);
  template<class T, uint Count>
  void CreatePerObjectUBO(api::CommandList& cmd);
  template<class T>
  void UpdatePerFrameUBO(api::CommandList& cmd, uint curr_frame, const T& pfcb);
  template<class T>
  void UpdatePerObjectUBO(api::CommandList& cmd, uint curr_frame, const std::vector<T>& pocbs);

public:
  constexpr static uint v_max_materials = 128;
  constexpr static uint v_max_objects = 128;
  constexpr static const char* v_dummy_image = "dummy_handle";

private:
  auto GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>;
  uint CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset);

private:
  api::Device& device_;  
  std::set<ModelHandle> models_;
  std::vector<api::Buffer*> staging_buffers_;
  api::ImageView* dummy_view_;
  std::vector<api::Buffer*> pocb_uniform_;
  std::vector<api::Buffer*> pocb_staging_;
  std::vector<api::BufferBarrier*> pocb_to_write_barriers_;
  std::vector<api::BufferBarrier*> pocb_to_read_barriers_;
  std::vector<api::Buffer*> pfcb_uniform_;
  std::vector<api::Buffer*> pfcb_staging_;
  std::vector<api::BufferBarrier*> pfcb_to_write_barriers_;
  std::vector<api::BufferBarrier*> pfcb_to_read_barriers_;

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

#include "scene_manager.inl"

#endif // GFX_VK_SCENE_MGR