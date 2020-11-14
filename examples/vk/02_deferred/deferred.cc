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
  std::vector<ModelHandle> models = helpers::LoadAbstractModels(cfg);

  api::CommandList setup_list = gfx.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  api::Fence submit_fence (device);
 
  SceneManager scene(gfx);
  scene.SetModels(models);
  
  uint vstg = scene.CreateStagingBuffer(MB(64));
  uint istg = scene.CreateStagingBuffer(MB(32));
  uint tstg = scene.CreateStagingBuffer(MB(96));
  
  scene.CopyGeometryToGpu(models, vstg, istg, setup_list);
  scene.CopyTexturesToGpu(models, tstg, setup_list);
  scene.CreateDummyView(setup_list);

  GbufferPass v_gbuffer_pass(gfx);
  v_gbuffer_pass.CreateUbo(setup_list, SceneManager::v_max_objects);
  v_gbuffer_pass.CreateImages(setup_list);
  v_gbuffer_pass.CreateRenderPass();
  v_gbuffer_pass.CreateFramebuffer();

  const api::ImageViews& rmat = scene.GetRenderableMaterials();
  v_gbuffer_pass.CreateDescriptorSet(rmat);
  v_gbuffer_pass.CreatePipeline();

  DeferredPass v_deferred_pass(gfx::v_num_images, gfx);
  for (uint i = 0; i < gfx::v_num_images; ++i)
    v_deferred_pass.CreateUbo(setup_list, i);
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
  camera.SetPos(Vec3f(0.f, 0.f, 0.f));
  camera.SetMoveSpeed(2.f);

  VkDevice vkdevice = gfx.GetDevice();
  VkPhysicalDevice physical_device = gfx.GetDevice();
  VkQueue queue = gfx.GetDevice().GetQueue(gfx::EQueueType::GRAPHICS);
  uint32_t queue_family = gfx.GetDevice().GetPhysicalDevice().queue_id;
  uint32_t nodes = 1;

  GDM_PROFILING_ENABLE();
  // MicroProfileGpuInitVulkan(&vkdevice, &physical_device, &queue, &queue_family, nodes);

  int exit = -1;
  while(exit == -1)
  {
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      exit = (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : -1;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // api::CommandList cmd_profiler = gfx.CreateCommandList(GDM_HASH("Profiler"), gfx::ECommandListFlags::SIMULTANEOUS);
 	  // MICROPROFILE_CONDITIONAL(MicroProfileThreadLogGpu* pGpuLog = MicroProfileThreadLogGpuAlloc());
	  // MICROPROFILE_GPU_BEGIN(cmd_profiler, pGpuLog);

    input.Capture();
    win.ProcessInput(input);
    timer.Start();
    float dt = timer.GetLastDt();
    helpers::UpdateCamera(camera, input, dt);

    // MICROPROFILE_GPU_SET_CONTEXT(&cmd_profiler, pGpuLog);
  	// MICROPROFILE_SCOPEGPUI("Full Frame", 0xff00ff00);
		// MICROPROFILE_SCOPEGPUI_L(pGpuLog, "DrawLoop", 0XFFFF);

    api::CommandList cmd_gbuffer = gfx.CreateCommandList(GDM_HASH("Gbuffer"), gfx::ECommandListFlags::SIMULTANEOUS);
  
    Mat4f view = camera.GetViewMx();
    Mat4f proj = camera.GetProjectionMx();
    v_gbuffer_pass.data_.pfcb_data_vs_.u_view_proj_ = proj * view;
    v_gbuffer_pass.data_.pfcb_data_vs_.u_cam_pos_ = camera.GetPos();

    uint mesh_number = 0;
    for (auto model_handle : scene.GetRenderableModels())
    {
      AbstractModel* model = ModelFactory::Get(model_handle);
      for (auto&& [i, mesh_handle] : Enumerate(model->meshes_))
      {
        AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
        AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
        v_gbuffer_pass.data_.pocb_data_vs_[mesh_number].u_model_ = model->tm_;
        v_gbuffer_pass.data_.pocb_data_ps_[mesh_number].u_material_index_ = material->index_;
        v_gbuffer_pass.data_.pocb_data_ps_[mesh_number].u_material_props_ = material->props_;
        ++mesh_number;
      }
    }
    v_gbuffer_pass.UpdateUbo(cmd_gbuffer, SceneManager::v_max_objects);
    v_gbuffer_pass.data_.descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *v_gbuffer_pass.data_.pfcb_uniform_vs_);
    v_gbuffer_pass.data_.descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, *v_gbuffer_pass.data_.pocb_uniform_vs_);
    v_gbuffer_pass.data_.descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(2, *v_gbuffer_pass.data_.pocb_uniform_ps_);
    v_gbuffer_pass.data_.descriptor_set_->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(4, scene.GetRenderableMaterials());
    v_gbuffer_pass.data_.descriptor_set_->Finalize();

    for(auto* barrier : v_gbuffer_pass.data_.image_barriers_to_write_)
      cmd_gbuffer.PushBarrier(*barrier);

    cmd_gbuffer.BindPipelineGraphics(*v_gbuffer_pass.pipeline_);
    cmd_gbuffer.BeginRenderPass(*v_gbuffer_pass.pass_, *v_gbuffer_pass.data_.fb_, width, height);

    mesh_number = 0;
    for (auto model_handle : scene.GetRenderableModels())
    {
      AbstractModel* model = ModelFactory::Get(model_handle);
      for (auto mesh_handle : model->meshes_)
      {
        AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
        uint offset = sizeof(GbufferVs_POCB) * mesh_number++;
        api::DescriptorSets descriptor_sets {*v_gbuffer_pass.data_.descriptor_set_};
        
        cmd_gbuffer.BindDescriptorSetGraphics(descriptor_sets, *v_gbuffer_pass.pipeline_, gfx::Offsets{offset, offset});      
        cmd_gbuffer.BindVertexBuffer(*mesh->GetVertexBuffer<api::Buffer>());
        cmd_gbuffer.BindIndexBuffer(*mesh->GetIndexBuffer<api::Buffer>());
        cmd_gbuffer.DrawIndexed(mesh->faces_);
      }
    }
    
    cmd_gbuffer.EndRenderPass();
    for(auto* barrier : v_gbuffer_pass.data_.image_barriers_to_read_)
      cmd_gbuffer.PushBarrier(*barrier);

    submit_fence.Reset();
    cmd_gbuffer.Finalize();
    gfx.SubmitCommandLists(api::CommandLists{cmd_gbuffer}, api::Semaphores::empty, api::Semaphores{sgbuffer_done}, submit_fence);
    submit_fence.WaitSignalFromGpu();

    uint curr_frame = gfx.AcquireNextFrame(spresent_done, api::Fence::null);
    api::CommandList cmd_deferred = gfx.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);

    DeferredPs_PFCB& deferred_pfcb = v_deferred_pass.data_[curr_frame].pfcb_data_ps_;
    deferred_pfcb.camera_pos_ = Vec4f(camera.GetPos(), 1.f);
    deferred_pfcb.global_ambient_ = Vec4f(0.2f);
    for(auto& light : deferred_pfcb.lights_)
    {
      light.color_ = color::LightYellow;
      light.dir_ = Vec4f(0.f,-1.f,1.f,0.f);
      light.enabled_ = true;
      light.type_ = LightType::DIR;
    }
    v_deferred_pass.UpdateUbo(cmd_deferred, curr_frame);
    cmd_deferred.PushBarrier(*v_deferred_pass.data_[curr_frame].present_to_write_barrier_);    

    api::DescriptorSets descriptor_sets {*v_deferred_pass.data_[curr_frame].descriptor_set_};
    cmd_deferred.BindDescriptorSetGraphics(descriptor_sets, *v_deferred_pass.pipeline_, gfx::Offsets{0});      
    cmd_deferred.BindPipelineGraphics(*v_deferred_pass.pipeline_);
    cmd_deferred.BeginRenderPass(*v_deferred_pass.pass_, *v_deferred_pass.data_[curr_frame].fb_, width, height);
    cmd_deferred.DrawDummy();
    cmd_deferred.EndRenderPass();
  
    cmd_deferred.PushBarrier(*v_deferred_pass.data_[curr_frame].present_to_read_barrier_);

    submit_fence.Reset();
    cmd_deferred.Finalize();

  	// MicroProfileFlip(&cmd_profiler);
    // cmd_profiler.Finalize();

    gfx.SubmitCommandLists(api::CommandLists{cmd_deferred}, api::Semaphores{sgbuffer_done}, api::Semaphores{sdeferred_done}, submit_fence);
    gfx.SubmitPresentation(curr_frame, api::Semaphores{spresent_done, sdeferred_done});
    submit_fence.WaitSignalFromGpu();

    timer.End();
    timer.Wait();  
    
  	// uint64_t index = MICROPROFILE_GPU_END(pGpuLog);
  }

  // GDM_PROFILER_SHUTDOWN();

  return static_cast<int>(msg.wParam);
}