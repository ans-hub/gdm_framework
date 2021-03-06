// *************************************************************
// File:    app_renderer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "app_renderer.h"

#include "system/diff_utils.h"
#include "system/literals.h"
#include "system/event_point.h"

#include "render/debug.h"

#include "app_helpers.h"
#include "app_defines.h"

// --public

gdm::AppRenderer::AppRenderer(
  api::Renderer& ctx,
  GpuStreamer& gpu_streamer,
  const DebugDraw& debug_draw
)
  : ctx_{ ctx }
  , device_{ ctx_.GetDevice() }
  , submit_fence_(device_)
  , setup_list_{ ctx_.CreateCommandList(GDM_HASH("AppRendererSetup"), gfx::ECommandListFlags::ONCE) }
  , gbuffer_pass_{
      ctx_,
      setup_list_,
      cfg::v_max_objects,
      cfg::v_max_materials * cfg::v_material_type_cnt}
  , deferred_pass_(gfx::v_num_images, ctx_)
  , debug_pass_(gfx::v_num_images, ctx_)
  , text_pass_(gfx::v_num_images, ctx_)
  , thread_command_lists_{}
  , thread_command_lists_cs_{}
  , gpu_events_{}
{
  text_pass_.BindFont(debug_draw.GetFont(), debug_draw.GetFontView());

  for (uint i = 0; i < gfx::v_num_images; ++i)
    deferred_pass_.CreateUniforms(setup_list_, i);

  deferred_pass_.CreateImages(setup_list_);
  deferred_pass_.CreateRenderPass();
  deferred_pass_.CreateFramebuffer();
  deferred_pass_.CreatePipeline(gbuffer_pass_.GetTextureViews());

  debug_pass_.CreateBarriers(setup_list_);
  debug_pass_.CreateRenderPass();
  debug_pass_.CreateFramebuffer();
  
  for (uint i = 0; i < gfx::v_num_images; ++i)
  {
    debug_pass_.CreateUniforms(setup_list_, i);
    debug_pass_.CreateVertexBuffer(setup_list_, i, 128_Kb);
  }
  
  debug_pass_.CreatePipeline();
  debug_pass_.CreateGui();
  
  text_pass_.CreateBarriers(setup_list_);
  text_pass_.CreateRenderPass();
  text_pass_.CreateFramebuffer();

  for (uint i = 0; i < gfx::v_num_images; ++i)
  {
    text_pass_.CreateUniforms(setup_list_, i);
    text_pass_.CreateVertexBuffer(setup_list_, i);
  }

  text_pass_.CreatePipeline();

  submit_fence_.Reset();

  setup_list_.Finalize();
  ctx_.SubmitCommandLists(api::CommandLists{setup_list_}, api::Semaphores::empty, api::Semaphores::empty, submit_fence_);
  submit_fence_.WaitSignalFromGpu();
  submit_fence_.Reset();
}

void gdm::AppRenderer::Render(
  float dt,
  const DebugDraw& debug_draw,
  const CameraEul& camera,
  const std::vector<ModelInstance*>& models,
  const api::ImageViews& materials,
  const std::vector<ModelLight>& lamps,
  const std::vector<ModelLight>& flashlights,
  const std::vector<GuiCallback>& gui_callbacks)
{
  std::vector<api::Renderer::Event> gpu_events;

  ctx_.FlushGpuEvents(gpu_events);
  ProcessGpuEvents(gpu_events);

  api::CommandList cmd_gbuffer = ctx_.CreateCommandList(GDM_HASH("Gbuffer"), gfx::ECommandListFlags::SIMULTANEOUS);

  api::Semaphore spresent_done(device_);
  api::Semaphore sgbuffer_done(device_);
  api::Semaphore sdeferred_done(device_);
  api::Semaphore sdebug_done(device_);

  gbuffer_pass_.UpdateUniformsData(camera, models);
  gbuffer_pass_.UpdateUniforms(cmd_gbuffer, cfg::v_max_objects);
  gbuffer_pass_.UpdateDescriptorSet(materials);
  gbuffer_pass_.Render(cmd_gbuffer, models);

  submit_fence_.Reset();
  cmd_gbuffer.Finalize();
  ctx_.SubmitCommandLists(api::CommandLists{cmd_gbuffer}, api::Semaphores::empty, api::Semaphores{sgbuffer_done}, submit_fence_);
  submit_fence_.WaitSignalFromGpu();

  uint curr_frame = ctx_.AcquireNextFrame(spresent_done, api::Fence::null);
  
  api::CommandList cmd_deferred = ctx_.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

  deferred_pass_.Update(curr_frame, camera, lamps, flashlights);
  deferred_pass_.Render(cmd_deferred, curr_frame);

  submit_fence_.Reset();
  cmd_deferred.Finalize();
 
  bool active_gui = false;
  
  for (auto&& cb : gui_callbacks)
    active_gui |= cb.active_;
 
  const bool active_wire = debug_draw.IsActiveWire();
  const bool active_text = debug_draw.IsActiveText();

  if (active_gui || active_wire || active_text)
  {
    ctx_.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence_);
    submit_fence_.WaitSignalFromGpu();
  
    api::CommandList cmd_debug = ctx_.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

    if (active_wire || active_gui)
    {
      debug_pass_.UpdateUniformsData(curr_frame, camera);
      debug_pass_.UpdateUniforms(cmd_debug, curr_frame);
      debug_pass_.UpdateVertexData(cmd_debug, curr_frame, debug_draw.GetDrawData());
      debug_pass_.Draw(cmd_debug, curr_frame, active_wire, active_gui, gui_callbacks);
    }

    if (active_text)
    {
      text_pass_.UpdateVertexData(cmd_debug, curr_frame, debug_draw.GetTextData());
      text_pass_.Draw(cmd_debug, curr_frame);
    }

    submit_fence_.Reset();
    cmd_debug.Finalize();

    ctx_.SubmitCommandLists(api::CommandLists{cmd_debug}, api::Semaphores{sdeferred_done}, api::Semaphores{sdebug_done}, submit_fence_);
    ctx_.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdebug_done});
    submit_fence_.WaitSignalFromGpu();
  }
  else
  {
    ctx_.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence_);
    ctx_.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdeferred_done});
    submit_fence_.WaitSignalFromGpu();
  }
}

// todo: for future submit from threads
void gdm::AppRenderer::EnqueueCommandList(const api::CommandList& cmd)
{
  std::unique_lock<std::mutex> lock(thread_command_lists_cs_);
  thread_command_lists_.push_back(cmd);
}

void gdm::AppRenderer::ProcessGpuEvents(std::vector<api::Renderer::Event>& gpu_events)
{
  for (auto&& gpu_event : gpu_events)
  {
    switch(gpu_event)
    {
      case api::Renderer::Event::SwapchainRecreated:
      {
        api::CommandList setup_list = ctx_.CreateCommandList(GDM_HASH("AppRendererSetup"), gfx::ECommandListFlags::ONCE);

        // todo: add sync - stop all threads
        gbuffer_pass_.Recreate(setup_list);

        deferred_pass_.CleanupPipeline();
        deferred_pass_.CreateImages(setup_list);
        deferred_pass_.CreateRenderPass();
        deferred_pass_.CreateFramebuffer();
        deferred_pass_.CreatePipeline(gbuffer_pass_.GetTextureViews());

        text_pass_.CleanupPipeline();
        text_pass_.CreateBarriers(setup_list);
        text_pass_.CreateRenderPass();
        text_pass_.CreateFramebuffer();

        for (uint i = 0; i < gfx::v_num_images; ++i)
        {
          text_pass_.CreateUniforms(setup_list, i);
          text_pass_.CreateVertexBuffer(setup_list, i);
        }
        text_pass_.CreatePipeline();

        debug_pass_.CleanupPipeline();
        debug_pass_.CreateBarriers(setup_list);
        debug_pass_.CreateRenderPass();
        debug_pass_.CreateFramebuffer();        
        debug_pass_.CreatePipeline();
      
        submit_fence_.Reset();
        setup_list.Finalize();
        ctx_.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence_);
        submit_fence_.WaitSignalFromGpu();
        submit_fence_.Reset();
      }


    }
  }
  gpu_events_.clear();
}