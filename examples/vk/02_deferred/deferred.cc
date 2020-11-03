// *************************************************************
// File:    deferred.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_VK_API
#define GFX_VK_API
#endif

#include "Windows.h"

#include "render/api.h"
#include "render/defines.h"
#include "render/renderer.h"

#include "render/input_layout.h"
#include "render/sampler_state.h"
#include "render/rasterizer_desc.h"
#include "render/viewport_desc.h"
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

#include "shaders/flat_vert.h"
#include "desc/std_rasterizer_desc.h"
#include "desc/std_input_layout.h"
#include "desc/std_sampler_desc.h"

#include "scene.h"

using namespace gdm;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  uint width = 800;
  uint height = 600;
  MainWindow win (width, height, "Vk Deferred", MainWindow::CENTERED);
  MainInput input (win.GetHandle(), hInstance);
  Renderer gfx(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);  
  ViewportDesc viewport{0, static_cast<float>(height), static_cast<float>(width), -static_cast<float>(height), 0, 1};

  api::Device& device = gfx.GetDevice();
  width = gfx.GetSurfaceWidth();
  height = gfx.GetSurfaceHeight();

  auto depth_image = api::Image2D(&device, width, height, gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT, gfx::EFormatType::D16_UNORM);
  auto depth_image_view = api::helpers::CreateImageView(device, depth_image.GetHandle(), depth_image.GetFormat());
  auto barrier = api::ImageBarrier(&device, depth_image.GetHandle(), gfx::EImageLayout::UNDEFINED, gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  api::Fence submit_fence (device);
  api::CommandList setup_list = gfx.CreateSetupCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  setup_list.PushBarrier(barrier);

  DataStorage<api::RenderPass> render_passes {};
  render_passes.Create(GDM_HASH("MainRenderPass"), device);

  api::RenderPass& render_pass = render_passes.Get(GDM_HASH("MainRenderPass"));
  uint color_idx = 0;
  uint depth_idx = 1;
  uint input_idx = -1;
  render_pass.AddPassDesccription(color_idx, gfx.GetSurfaceFormat(), gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);
  render_pass.AddPassDesccription(depth_idx, depth_image.GetFormat<gfx::EFormatType>(), gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  uint subpass_idx = render_pass.CreateSubpass(gfx::EQueueType::GRAPHICS);
  render_pass.AddSubpassColorAttachments(subpass_idx, api::Attachments{color_idx});
  render_pass.AddSubpassDepthAttachments(subpass_idx, api::Attachment{depth_idx});
  render_pass.Finalize();

  DataStorage<api::FrameBuffer> framebuffers {};
  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    api::ImageViews image_views {gfx.GetBackBufferViews()[i], depth_image_view};
    framebuffers.Create(GDM_HASH_N("MainFB", i), gfx.GetDevice(), width, height, render_pass, image_views);
  }

  DataStorage<Shader> shaders {};
  shaders.Create(GDM_HASH("FlatVx"), "shaders/flat_vert.hlsl", gfx::EShaderType::VX);
  shaders.Create(GDM_HASH("FlatPx"), "shaders/flat_frag.hlsl", gfx::EShaderType::PX);

  Config cfg("../../_configs/simple.cfg");
  ModelFactory::SetPath("../../_models_new/models/");
  MaterialFactory::SetPath("../../_models_new/materials/");
  TextureFactory::SetPath("../../_models_new/textures/");
  ImageFactory::SetPath("../../_models_new/textures/");

  Scene scene(gfx.GetDevice());
  std::vector<ModelHandle> models = scene.LoadAbstractModels(cfg);
  api::Buffer vstg (&device, MB(16), gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  api::Buffer istg (&device, MB(16), gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  api::Buffer tstg (&device, MB(16), gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  scene.CopyGeometryToGpu(models, vstg, istg, setup_list);
  scene.CopyTexturesToGpu(models, tstg, setup_list);
  
  std::vector<api::Buffer*> pocb_uniform;
  std::vector<api::Buffer*> pocb_staging;
  std::vector<api::BufferBarrier*> pocb_to_write_barriers;
  std::vector<api::BufferBarrier*> pocb_to_read_barriers;
  std::vector<api::Buffer*> pfcb_uniform;
  std::vector<api::Buffer*> pfcb_staging;
  std::vector<api::BufferBarrier*> pfcb_to_write_barriers;
  std::vector<api::BufferBarrier*> pfcb_to_read_barriers;

  constexpr uint v_max_objects = 32;

  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    auto* pocb_buf = GMNew api::Buffer(&device, sizeof(FlatVs_POCB) * v_max_objects, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
    pocb_uniform.push_back(pocb_buf);
    auto* pocb_staging_buf = GMNew api::Buffer(&device, sizeof(FlatVs_POCB) * v_max_objects, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE); // and COHERENT too?
    pocb_staging.push_back(pocb_staging_buf);
    auto* pocb_to_read_barrier = GMNew api::BufferBarrier(&device, *pocb_buf, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
    pocb_to_read_barriers.push_back(pocb_to_read_barrier);
    auto* pocb_to_write_barrier = GMNew api::BufferBarrier(&device, *pocb_buf, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
    pocb_to_write_barriers.push_back(pocb_to_write_barrier);

    pocb_staging[i]->Map();
    setup_list.PushBarrier(*pocb_to_read_barriers[i]);

    auto* pfcb_buf = GMNew api::Buffer(&device, sizeof(FlatVs_PFCB), gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
    pfcb_uniform.push_back(pfcb_buf);
    auto* pfcb_staging_buf = GMNew api::Buffer(&device, sizeof(FlatVs_PFCB), gfx::TRANSFER_SRC, gfx::HOST_VISIBLE); // and COHERENT too?
    pfcb_staging.push_back(pfcb_staging_buf);
    auto* pfcb_to_read_barrier = GMNew api::BufferBarrier(&device, *pfcb_buf, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
    pfcb_to_read_barriers.push_back(pfcb_to_read_barrier);
    auto* pfcb_to_write_barrier = GMNew api::BufferBarrier(&device, *pfcb_buf, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
    pfcb_to_write_barriers.push_back(pfcb_to_write_barrier);

    setup_list.PushBarrier(*pfcb_to_read_barriers[i]);
  }  

  setup_list.Finalize();
  gfx.SubmitCommandLists(api::CommandLists{setup_list}, api::Semaphores::empty, api::Semaphores::empty, submit_fence);
  submit_fence.WaitSignalFromGpu();
  submit_fence.Reset();

  api::Sampler sampler(device, StdSamplerState{});

  api::ImageViews material_diffuse_image_views;
  for (auto model_handle : scene.GetModels())
  {
    auto model = ModelFactory::Get(model_handle);
    for (auto mesh_handle : model->meshes_)
    {
      auto mesh = MeshFactory::Get(mesh_handle);
      auto material = MaterialFactory::Get(mesh->material_);
      auto diffuse_texture = TextureFactory::Get(material->diff_);
      arr_utils::EnsureIndex(material_diffuse_image_views, material->index_);
      material_diffuse_image_views[material->index_] = *diffuse_texture->GetImageView<api::ImageView>();
    }
  }

  auto* descriptor_layout = GMNew api::DescriptorSetLayout(device);
  descriptor_layout->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  descriptor_layout->AddBinding(1, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::VERTEX_STAGE);
  descriptor_layout->AddBinding(2, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  descriptor_layout->AddBinding(3, static_cast<uint>(material_diffuse_image_views.size()), gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::EBindingFlags::VARIABLE_DESCRIPTOR);
  descriptor_layout->Finalize();

  std::vector<api::DescriptorSet*> frame_descriptor_sets;
  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(device, *descriptor_layout, gfx.GetDescriptorPool());
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *pfcb_uniform[i]);
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, *pocb_uniform[i]);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(2, sampler);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(3, material_diffuse_image_views);
    descriptor_set->Finalize();
    frame_descriptor_sets.push_back(descriptor_set);
  }

  DataStorage<api::Pipeline> pipelines {};
  pipelines.Create(GDM_HASH("PipelineName"), gfx.GetDevice());
  api::Pipeline& pipeline = pipelines.Get(GDM_HASH("PipelineName"));

  pipeline.SetShaderStage(shaders.Get(GDM_HASH("FlatVx")), gfx::EShaderType::VX);
  pipeline.SetShaderStage(shaders.Get(GDM_HASH("FlatPx")), gfx::EShaderType::PX);
  pipeline.SetViewportState(viewport);
  pipeline.SetRasterizerState(StdRasterizerDesc{});
  pipeline.SetInputLayout(StdInputLayout{});
  pipeline.SetRenderPass(render_pass);
  pipeline.SetDescriptorSetLayouts(api::DescriptorSetLayouts{*descriptor_layout});
  pipeline.Compile();

  MSG msg {0};
  Timer timer {60};
  FpsCounter fps {};

  FlatVs_POCB pocb {Mat4f(1.f), Vec4f{0.5f, 0.5f, 0.5f, 0.5f}};
  FlatVs_PFCB pfcb {};

  api::Semaphore spresent_done(device);
  api::Semaphore srender_done(device);

  auto present_images = gfx.GetBackBufferImages();
  std::vector<api::ImageBarrier*> present_to_read_barrier = {};
  std::vector<api::ImageBarrier*> present_to_write_barrier = {};

  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    auto* barrier_to_read = GMNew api::ImageBarrier(
      &device, present_images[i], gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL, gfx::EImageLayout::PRESENT_SRC);
    auto* barrier_to_write = GMNew api::ImageBarrier(
      &device, present_images[i], gfx::EImageLayout::PRESENT_SRC, gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);
    present_to_read_barrier.push_back(barrier_to_read);
    present_to_write_barrier.push_back(barrier_to_write);
  }

  CameraEul camera(75.f, win.GetAspectRatio(), 0.1f, 100.f);
  camera.SetPos(Vec3f(0.f, 0.f, 0.f));
  camera.SetMoveSpeed(2.f);

  int exit = -1;
  while(exit == -1)
  {
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      exit = (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : -1;
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    timer.Start();
    float dt = timer.GetLastDt();
    input.Capture();
    win.ProcessInput(input);
    
    uint curr_frame = gfx.AcquireNextFrame(spresent_done, api::Fence::null);
    api::CommandList cmd = gfx.CreateFrameCommandList(curr_frame, gfx::ECommandListFlags::SIMULTANEOUS);
    cmd.PushBarrier(*present_to_write_barrier[curr_frame]);

    scene.UpdateCamera(camera, input, dt);
    Mat4f view = camera.GetViewMx();
    Mat4f proj = camera.GetProjectionMx();
    pfcb.u_view_proj_ = proj * view;

    cmd.PushBarrier(*pfcb_to_write_barriers[curr_frame]);
    pfcb_staging[curr_frame]->Map();
    pfcb_staging[curr_frame]->CopyDataToGpu(&pfcb, 1);
    pfcb_staging[curr_frame]->Unmap();
    cmd.CopyBufferToBuffer(*pfcb_staging[curr_frame], *pfcb_uniform[curr_frame], sizeof(FlatVs_PFCB));
    cmd.PushBarrier(*pfcb_to_read_barriers[curr_frame]);

    std::vector<FlatVs_POCB> pocbs;
    for (auto model_handle : scene.GetModels())
    {
      AbstractModel* model = ModelFactory::Get(model_handle);
      FlatVs_POCB pocb_curr = {};
      pocb_curr.u_model_ = model->tm_;
      pocbs.push_back(pocb_curr);
    }

    cmd.PushBarrier(*pocb_to_write_barriers[curr_frame]);
    pocb_staging[curr_frame]->CopyDataToGpu(pocbs.data(), pocbs.size());
    uint pocb_size = static_cast<uint>(sizeof(FlatVs_POCB) * pocbs.size());
    cmd.CopyBufferToBuffer(*pocb_staging[curr_frame], *pocb_uniform[curr_frame], pocb_size);
    cmd.PushBarrier(*pocb_to_read_barriers[curr_frame]);

    auto& fb = framebuffers.Get(GDM_HASH_N("MainFB", curr_frame));
    cmd.BindPipelineGraphics(pipeline); // scene.GetPipeline(GDM_HASH("dsa"));

    cmd.BeginRenderPass(render_pass, fb, width, height);

    uint model_number = 0;
    for (auto model_handle : scene.GetModels())
    {
      AbstractModel* model = ModelFactory::Get(model_handle);
      
      uint offset = model_number * sizeof(FlatVs_POCB);
      cmd.BindDescriptorSetGraphics(api::DescriptorSets{*frame_descriptor_sets[curr_frame]}, pipeline, gfx::Offsets{offset});
      
      for (auto mesh_handle : model->meshes_)
      {
        AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
        AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
        AbstractTexture* texture = TextureFactory::Get(material->diff_);

        cmd.BindVertexBuffer(*mesh->GetVertexBuffer<api::Buffer>());
        cmd.BindIndexBuffer(*mesh->GetIndexBuffer<api::Buffer>());
        cmd.DrawIndexed(mesh->faces_);
      }
      ++model_number;
    }
    
    cmd.EndRenderPass();
    cmd.PushBarrier(*present_to_read_barrier[curr_frame]);
    cmd.Finalize();

    api::Fence submit_fence(device);
    gfx.SubmitCommandLists(api::CommandLists{cmd}, api::Semaphores{spresent_done}, api::Semaphores{srender_done}, submit_fence);
    gfx.SubmitPresentation(curr_frame, api::Semaphores{srender_done});
    submit_fence.WaitSignalFromGpu();

    timer.End();
    timer.Wait();  
  }

  return static_cast<int>(msg.wParam);
}