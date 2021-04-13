// *************************************************************
// File:    app_renderer.h
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

#ifndef GFX_PLAYGROUND_RENDERER_H
#define GFX_PLAYGROUND_RENDERER_H

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/desc/viewport_desc.h"

#include "render/deferred_pass.h"
#include "render/gbuffer_pass.h"
#include "render/debug_pass.h"
#include "render/text_pass.h"

#include "window/main_input.h"
#include "data/model_factory.h"
#include "data/cfg_loader.h"
#include "system/font.h"

#include "engine/gui_manager.h"
#include "engine/gpu_streamer.h"

namespace gdm {

struct PlaygroundRenderer
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

  PlaygroundRenderer(api::Renderer& gfx, GpuStreamer& gpu_streamer, const DebugDraw& debug_draw);

  void Render(float dt,
              const DebugDraw& debug_draw,
              const CameraEul& camera,
              const std::vector<ModelInstance*>& models,
              const api::ImageViews& materials,
              const std::vector<ModelLight>& lamps,
              const std::vector<ModelLight>& flashlights,
              const std::vector<GuiCallback>& gui_callbacks);

  auto GetGpuInfo() const -> const api::PhysicalDevice& { return device_.GetPhysicalDevice().info_; }

private:
  api::Renderer& gfx_;
  api::Device& device_;
  api::Fence submit_fence_;

  std::vector<bool> stage_active_;

  GbufferPass gbuffer_pass_;
  DeferredPass deferred_pass_;
  DebugPass debug_pass_;
  TextPass text_pass_;

};  // struct PlaygroundRenderer

} // namespace gdm

#endif // GFX_PLAYGROUND_RENDERER_H