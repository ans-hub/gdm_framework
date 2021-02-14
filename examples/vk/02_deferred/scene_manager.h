// *************************************************************
// File:    scene_manager.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

/*
                    VS        PS        OUT
#1 pass gbuffer    PFUBO   tex_diff   out_gpos
        mrt        POUBO   tex_norm   out_gnorm
        offscreen                     out_gdiff
                                      out_depth

#2 pass deferred      -    PFUBO[2]   backbuffer[2]
        lighting      -    tex_gpos   depth[2]
        main               tex_gdiff
                           tex_gnorm
*/

#ifndef GFX_VK_SCENE_MGR
#define GFX_VK_SCENE_MGR

#include <string>

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/viewport_desc.h"

#include "data/model_factory.h"

namespace gdm {

struct SceneManager
{
  struct DeferredPass;
  struct GbufferPass;

  SceneManager(api::Renderer& gfx);

  void CreateDummyView(api::CommandList& cmd);
  uint CreateStagingBuffer(uint bytes);
  auto GetStagingBuffer(uint index) -> api::Buffer& { return *staging_buffers_[index]; };
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, uint vstg_index, uint istg_index, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<ModelHandle>& models, uint tstg_index, api::CommandList& list);
  void SetObjects(const std::vector<ModelInstance>& objs, const std::vector<std::string>& names);
  void SetLamps(const std::vector<ModelInstance>& lamps, const std::vector<ModelInstance>& flashlights);
  auto GetRenderableInstances() -> std::vector<ModelInstance*>;
  auto GetSceneInstances() -> std::vector<ModelInstance*>;
  auto GetSceneInstancesNames() -> std::vector<std::string>&;
  auto GetRenderableMaterials() -> const api::ImageViews&;
  auto GetLamps() -> std::vector<ModelLight>& { return lamps_; }
  auto GetFlashlights() -> std::vector<ModelLight>& { return flashlights_; }

public:
  constexpr static uint v_material_type_cnt = 3; // diff_map + norm_map + v_spec_map
  constexpr static uint v_max_materials = 32;
  constexpr static uint v_diff_offset = 0;
  constexpr static uint v_norm_offset = 1;
  constexpr static uint v_spec_offset = 2;
  constexpr static uint v_max_lights = 8;
  constexpr static uint v_max_objects = 512;
  constexpr static const char* v_dummy_image = "dummy_handle";

private:
  auto GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>;
  uint CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset);

private:
  api::Device& device_;
  api::Renderer& rdr_;
  std::vector<ModelInstance> models_;
  std::vector<std::string> models_names_;
  std::vector<ModelLight> flashlights_;
  std::vector<ModelLight> lamps_;
  api::ImageViews renderable_materials_;
  std::vector<api::Buffer*> staging_buffers_;
  api::ImageView* dummy_view_;

private:
  static constexpr const char* v_model_prefix = "model_";
  static constexpr const char* v_model_pos_prefix = "model_pos_";

};  // struct SceneManager

} // namespace gdm::scene

#endif // GFX_VK_SCENE_MGR