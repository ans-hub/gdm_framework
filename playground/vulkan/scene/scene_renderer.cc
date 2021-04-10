// *************************************************************
// File:    scene_renderer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene_renderer.h"

#include "system/diff_utils.h"
#include "system/literals.h"
#include "system/event_point.h"

#include "render/debug.h"
#include "render/shader.h"
#include "render/desc/sampler_desc.h"
#include "render/desc/input_layout_desc.h"
#include "render/desc/rasterizer_desc.h"

#include "data_helpers.h"
#include "defines.h"

// --public

gdm::SceneRenderer::SceneRenderer(api::Renderer& gfx, GpuStreamer& gpu_streamer)
  : gfx_{ gfx }
  , device_{ gfx_.GetDevice() }
  , submit_fence_(device_)
  , debug_draw_{}
  , gui_draw_{}
  , stage_active_(static_cast<int>(EStage::Max), true)
  , gbuffer_pass_(gfx_)
  , deferred_pass_(gfx::v_num_images, gfx_)
  , debug_pass_(gfx::v_num_images, gfx_)
  , text_pass_(gfx::v_num_images, gfx_)
{
  debug_draw_.ToggleActive();
  debug_draw_.AddFont(gpu_streamer, "assets/fonts/arial.ttf", 14);
  gui_draw_.ToggleActive(GuiDraw::EStage::BG_TEXT);
  debug_pass_.AddGuiCallback(gui::GuiExampleCb);
  text_pass_.BindFont(debug_draw_.GetFont(), debug_draw_.GetFontView());

  api::CommandList setup_list = gfx_.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);

  gbuffer_pass_.CreateUniforms(setup_list, cfg::v_max_objects);
  gbuffer_pass_.CreateImages(setup_list);
  gbuffer_pass_.CreateRenderPass();
  gbuffer_pass_.CreateFramebuffer();
  gbuffer_pass_.CreateDescriptorSet(cfg::v_max_materials * cfg::v_material_type_cnt);
  gbuffer_pass_.CreatePipeline();

  for (uint i = 0; i < gfx::v_num_images; ++i)
    deferred_pass_.CreateUniforms(setup_list, i);

  deferred_pass_.CreateImages(setup_list);
  deferred_pass_.CreateRenderPass();
  deferred_pass_.CreateFramebuffer();
  deferred_pass_.CreatePipeline(gbuffer_pass_.data_.image_views_);

  debug_pass_.CreateBarriers(setup_list);
  debug_pass_.CreateRenderPass();
  debug_pass_.CreateFramebuffer();
  
  for (uint i = 0; i < gfx::v_num_images; ++i)
  {
    debug_pass_.CreateUniforms(setup_list, i);
    debug_pass_.CreateVertexBuffer(setup_list, i, 128_Kb);
  }
  
  debug_pass_.CreatePipeline();
  debug_pass_.CreateGui();
  
  text_pass_.CreateBarriers(setup_list);
  text_pass_.CreateRenderPass();
  text_pass_.CreateFramebuffer();

  for (uint i = 0; i < gfx::v_num_images; ++i)
  {
    text_pass_.CreateUniforms(setup_list, i);
    text_pass_.CreateVertexBuffer(setup_list, i);
  }

  text_pass_.CreatePipeline();

  submit_fence_.Reset();

  setup_list.Finalize();
  gfx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence_);
  submit_fence_.WaitSignalFromGpu();
  submit_fence_.Reset();
}

void gdm::SceneRenderer::ToggleActive(EStage stage)
{
  stage_active_[static_cast<int>(stage)] = !stage_active_[static_cast<int>(stage)];
}

void gdm::SceneRenderer::SetActive(EStage stage, bool is_active)
{
  stage_active_[static_cast<int>(stage)] = is_active;
}

void gdm::SceneRenderer::Render(
  float dt,
  const CameraEul& camera,
  const std::vector<ModelInstance*>& models,
  const api::ImageViews& materials,
  std::vector<ModelLight>& lamps,
  std::vector<ModelLight>& flashlights)
{
  SetActive(EStage::TEXT, gui_draw_.IsActive(GuiDraw::EStage::BG_TEXT));
  SetActive(EStage::GUI, gui_draw_.IsActive(GuiDraw::EStage::WINDOWS));
  SetActive(EStage::DEBUG, debug_draw_.IsActive());

  api::CommandList cmd_gbuffer = gfx_.CreateCommandList(GDM_HASH("Gbuffer"), gfx::ECommandListFlags::SIMULTANEOUS);

  api::Semaphore spresent_done(device_);
  api::Semaphore sgbuffer_done(device_);
  api::Semaphore sdeferred_done(device_);
  api::Semaphore sdebug_done(device_);

  gbuffer_pass_.UpdateUniformsData(camera, models);
  gbuffer_pass_.UpdateUniforms(cmd_gbuffer, cfg::v_max_objects);
  gbuffer_pass_.UpdateDescriptorSet(materials);
  gbuffer_pass_.Draw(cmd_gbuffer, models);

  submit_fence_.Reset();
  cmd_gbuffer.Finalize();
  gfx_.SubmitCommandLists(api::CommandLists{cmd_gbuffer}, api::Semaphores::empty, api::Semaphores{sgbuffer_done}, submit_fence_);
  submit_fence_.WaitSignalFromGpu();

  uint curr_frame = gfx_.AcquireNextFrame(spresent_done, api::Fence::null);
  
  api::CommandList cmd_deferred = gfx_.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

  deferred_pass_.UpdateUniformsData(curr_frame, camera, lamps, flashlights);
  deferred_pass_.UpdateUniforms(cmd_deferred, curr_frame);
  deferred_pass_.Draw(cmd_deferred, curr_frame);

  submit_fence_.Reset();
  cmd_deferred.Finalize();

  const bool any_debug = IsStageActive(EStage::DEBUG) || IsStageActive(EStage::GUI) || IsStageActive(EStage::TEXT);

  if (any_debug)
  {
    gfx_.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence_);
    submit_fence_.WaitSignalFromGpu();
  
    api::CommandList cmd_debug = gfx_.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

    if (IsStageActive(EStage::DEBUG) || IsStageActive(EStage::GUI))
    {
      debug_pass_.UpdateUniformsData(curr_frame, camera);
      debug_pass_.UpdateUniforms(cmd_debug, curr_frame);
      debug_pass_.UpdateVertexData(cmd_debug, curr_frame, debug_draw_.GetDrawData());
      debug_pass_.Draw(cmd_debug, curr_frame, IsStageActive(EStage::DEBUG), IsStageActive(EStage::GUI));
    }

    if (IsStageActive(EStage::TEXT))
    {
      text_pass_.UpdateVertexData(cmd_debug, curr_frame, debug_draw_.GetTextData());
      text_pass_.Draw(cmd_debug, curr_frame);
    }

    submit_fence_.Reset();
    cmd_debug.Finalize();

    gfx_.SubmitCommandLists(api::CommandLists{cmd_debug}, api::Semaphores{sdeferred_done}, api::Semaphores{sdebug_done}, submit_fence_);
    gfx_.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdebug_done});
    submit_fence_.WaitSignalFromGpu();
  }
  else
  {
    gfx_.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence_);
    gfx_.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdeferred_done});
    submit_fence_.WaitSignalFromGpu();
  }

  debug_draw_.Clear();
}
