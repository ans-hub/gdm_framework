// *************************************************************
// File:    deferred.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_API
#define GFX_VK_API
#endif

#include "Windows.h"

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"

#include "render/colors.h"
#include "render/shader.h"
#include "render/camera_eul.h"

#include "memory/defines.h"

#include "window/main_window.h"
#include "window/main_input.h"

#include "data/data_storage.h"
#include "data/model_factory.h"

#include "system/fps_counter.h"
#include "system/hash_utils.h"
#include "system/array_utils.h"
#include "system/timer.h"
#include "system/diff_utils.h"
#include "system/profiler.h"

#include "desc/rasterizer_desc.h"
#include "desc/input_layout.h"
#include "desc/sampler_desc.h"
#include "desc/deferred_pass.h"
#include "desc/gbuffer_pass.h"

#include "scene_manager.h"
#include "helpers.h"

using namespace gdm;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  uint width = 800;
  uint height = 600;
  MainWindow win (width, height, "Vk Deferred", MainWindow::CENTERED);
  MainInput input (win.GetHandle(), hInstance);
  Renderer gfx(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);  

  api::Device& device = gfx.GetDevice();
  width = gfx.GetSurfaceWidth();
  height = gfx.GetSurfaceHeight();

  ENSUREF(wcslen(cmdLine) != 0, "Config file name is empty");

  std::string cfg_name = str::Utf2Ansi(cmdLine);
  Config cfg(cfg_name.c_str());

  api::CommandList setup_list = gfx.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  api::Fence submit_fence (device);
 
  SceneManager scene(gfx);
  std::vector<ModelInstance> object_models = helpers::LoadObjects(cfg);
  std::vector<ModelInstance> lamp_models = helpers::LoadLights(cfg);
  std::vector<ModelInstance> flashlights = helpers::LoadFlashlights(cfg);
  scene.SetModels(object_models, lamp_models, flashlights);

  uint vstg = scene.CreateStagingBuffer(MB(64));
  uint istg = scene.CreateStagingBuffer(MB(32));
  uint tstg = scene.CreateStagingBuffer(MB(96));
  
  std::vector<ModelHandle> unique_models = helpers::GetUniqueModels(object_models, lamp_models);
  scene.CopyGeometryToGpu(unique_models, vstg, istg, setup_list);
  scene.CopyTexturesToGpu(unique_models, tstg, setup_list);
  scene.CreateDummyView(setup_list);

  GbufferPass v_gbuffer_pass(gfx);
  v_gbuffer_pass.CreateUniforms(setup_list, SceneManager::v_max_objects);
  v_gbuffer_pass.CreateImages(setup_list);
  v_gbuffer_pass.CreateRenderPass();
  v_gbuffer_pass.CreateFramebuffer();

  const api::ImageViews& rmat = scene.GetRenderableMaterials();
  v_gbuffer_pass.CreateDescriptorSet(rmat);
  v_gbuffer_pass.CreatePipeline();

  DeferredPass v_deferred_pass(gfx::v_num_images, gfx);
  for (uint i = 0; i < gfx::v_num_images; ++i)
    v_deferred_pass.CreateUniforms(setup_list, i);
  v_deferred_pass.CreateImages(setup_list);
  v_deferred_pass.CreateRenderPass();
  v_deferred_pass.CreateFramebuffer();
  v_deferred_pass.CreatePipeline(v_gbuffer_pass.data_.image_views_);

  submit_fence.Reset();
  api::Semaphore spresent_done(device);
  api::Semaphore sgbuffer_done(device);
  api::Semaphore sdeferred_done(device);

  setup_list.Finalize();
  gfx.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence);
  submit_fence.WaitSignalFromGpu();
  submit_fence.Reset();

  MSG msg {0};
  Timer timer {60};
  FpsCounter fps {};

  CameraEul camera(75.f, win.GetAspectRatio(), 0.1f, 100.f);
  camera.SetPos(cfg.Get<Vec3f>("initial_cam_pos"));
  camera.LookAt(cfg.Get<Vec3f>("initial_look_at"));
  camera.SetMoveSpeed(3.f);

  GDM_PROFILING_ENABLE();

  int exit = -1;
  while(exit == -1)
  {
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      exit = (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : -1;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    input.Capture();
    win.ProcessInput(input);
    timer.Start();
    float dt = timer.GetLastDt();
    helpers::UpdateCamera(camera, input, dt);
    helpers::UpdateLamps(camera, input, scene.GetLamps(), dt);
    helpers::UpdateObjects(scene.GetRenderableInstances(), dt);
    helpers::UpdateFlashlights(camera, input, scene.GetFlashlights(), dt);

    api::CommandList cmd_gbuffer = gfx.CreateCommandList(GDM_HASH("Gbuffer"), gfx::ECommandListFlags::SIMULTANEOUS);
  
    v_gbuffer_pass.UpdateUniformsData(camera, scene.GetRenderableInstances());
    v_gbuffer_pass.UpdateUniforms(cmd_gbuffer, SceneManager::v_max_objects);
    v_gbuffer_pass.UpdateDescriptorSet(scene.GetRenderableMaterials());
    v_gbuffer_pass.Draw(cmd_gbuffer, scene.GetRenderableInstances());

    submit_fence.Reset();
    cmd_gbuffer.Finalize();
    gfx.SubmitCommandLists(api::CommandLists{cmd_gbuffer}, api::Semaphores::empty, api::Semaphores{sgbuffer_done}, submit_fence);
    submit_fence.WaitSignalFromGpu();

    uint curr_frame = gfx.AcquireNextFrame(spresent_done, api::Fence::null);
    api::CommandList cmd_deferred = gfx.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

    v_deferred_pass.UpdateUniformsData(curr_frame, camera, scene.GetLamps(), scene.GetFlashlights());
    v_deferred_pass.UpdateUniforms(cmd_deferred, curr_frame);
    v_deferred_pass.Draw(cmd_deferred, curr_frame);

    submit_fence.Reset();
    cmd_deferred.Finalize();

    gfx.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence);
    gfx.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdeferred_done});
    submit_fence.WaitSignalFromGpu();

    timer.End();
    timer.Wait();  
  }

  return static_cast<int>(msg.wParam);
}