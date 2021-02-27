// *************************************************************
// File:    deferred_renderer.h
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

#ifndef GFX_VK_DEFERRED_RDR
#define GFX_VK_DEFERRED_RDR

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/desc/viewport_desc.h"

#include "passes/deferred_pass.h"
#include "passes/gbuffer_pass.h"
#include "passes/debug_pass.h"

#include "data/model_factory.h"
#include "data/cfg_loader.h"

#include "scene.h"

namespace gdm {

struct DeferredRenderer
{
  DeferredRenderer(HWND window_handle, gfx::DeviceProps flags = 0);
  void Setup(Scene& scene);
  void Update(Scene& scene);
  auto GetDebugDraw() -> DebugDraw& { return debug_draw_; }

private:
  uint CreateStagingBuffer(uint bytes);
  auto GetStagingBuffer(uint index) -> api::Buffer& { return *staging_buffers_[index]; };
  void CopyGeometryToGpu(const std::vector<ModelHandle>& models, uint vstg_index, uint istg_index, api::CommandList& list);
  void CopyTexturesToGpu(const std::vector<ModelHandle>& models, uint tstg_index, api::CommandList& list);
  void CreateDummyView(api::CommandList& cmd);

private:
  auto GetMaterialsToLoad(const std::vector<ModelHandle>& handles) -> std::vector<MaterialHandle>;
  uint CopyTextureToStagingBuffer(AbstractTexture* texture, api::Buffer& stg, uint curr_offset);
  void CopyTextureFromStagingBuffer(api::CommandList& cmd, AbstractTexture* texture, api::Buffer& stg, uint curr_offset);

private:
  constexpr static uint v_max_objects = 512;
  constexpr static const char* v_dummy_image = "dummy_handle";

private:
  api::Renderer gfx_;
  api::Device& device_;
  api::Fence submit_fence_;
  std::vector<api::Buffer*> staging_buffers_;

  GbufferPass gbuffer_pass_;
  DeferredPass deferred_pass_;
  DebugPass debug_pass_;

  DebugDraw debug_draw_;

};  // struct DeferredRenderer

} // namespace gdm::scene

#endif // GFX_VK_DEFERRED_RDR