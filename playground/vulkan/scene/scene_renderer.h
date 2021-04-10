// *************************************************************
// File:    scene_renderer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

/* Deferred renderer

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

#ifndef GFX_SCENE_RENDERER_H
#define GFX_SCENE_RENDERER_H

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/desc/viewport_desc.h"

#include "passes/deferred_pass.h"
#include "passes/gbuffer_pass.h"
#include "passes/debug_pass.h"
#include "passes/text_pass.h"

#include "window/main_input.h"

#include "data/model_factory.h"
#include "data/cfg_loader.h"

#include "system/font.h"

#include "gpu_streamer.h"
#include "debug_draw.h"
#include "gui_draw.h"

namespace gdm {

struct SceneRenderer
{
  enum class EStage
  {
    GBUFFER,
    DEFERRED,
    DEBUG,
    TEXT,
    GUI,
    Max
  }; // enum class EStage

  SceneRenderer(api::Renderer& gfx, GpuStreamer& gpu_streamer);

  void Render(float dt,
              const CameraEul& camera,
              const std::vector<ModelInstance*>& models,
              const api::ImageViews& materials,
              std::vector<ModelLight>& lamps,
              std::vector<ModelLight>& flashlights);

  void SetActive(EStage stage, bool is_active);
  void ToggleActive(EStage stage);
  auto IsStageActive(EStage stage) const -> int { return stage_active_[static_cast<int>(stage)]; };

  auto GetDebugDraw() -> DebugDraw& { return debug_draw_; }
  auto GetGuiDraw() -> GuiDraw& { return gui_draw_; }
  auto GetGpuInfo() const -> const api::PhysicalDevice& { return device_.GetPhysicalDevice().info_; }

private:
  api::Renderer& gfx_;
  api::Device& device_;
  api::Fence submit_fence_;

  DebugDraw debug_draw_;
  GuiDraw gui_draw_;

  std::vector<bool> stage_active_;

  GbufferPass gbuffer_pass_;
  DeferredPass deferred_pass_;
  DebugPass debug_pass_;
  TextPass text_pass_;

};  // struct Renderer

} // namespace gdm

#endif // GFX_SCENE_RENDERER_H