// *************************************************************
// File:    scene.cc
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
#include "render/desc/rasterizer_desc.h"
#include "render/desc/viewport_desc.h"
#include "render/colors.h"
#include "render/shader.h"
#include "render/camera_eul.h"

#include "window/main_window.h"
#include "window/main_input.h"

#include "data/data_storage.h"
#include "data/model_factory.h"

#include "system/fps_counter.h"
#include "system/hash_utils.h"
#include "system/timer.h"

#include "shaders/flat_vert.h"
#include "desc/rasterizer_desc.h"
#include "desc/input_layout.h"
#include "desc/sampler_desc.h"

#include "helpers.h"

using namespace gdm;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  uint width = 800;
  uint height = 600;
  MainWindow win (width, height, "Vk scene", MainWindow::CENTERED);
  MainInput input (win.GetHandle(), hInstance);
  Renderer gfx(win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);  
  ViewportDesc viewport{0, static_cast<float>(height), static_cast<float>(width), -static_cast<float>(height), 0, 1};

  api::Device& device = gfx.GetDevice();
  width = gfx.GetSurfaceWidth();
  height = gfx.GetSurfaceHeight();

  auto depth_image = api::Image2D(&device, width, height, gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT, gfx::EFormatType::D16_UNORM);
  auto depth_image_view = api::ImageView(device, depth_image.GetHandle(), depth_image.GetFormat());
  auto barrier = api::ImageBarrier(&device, depth_image.GetHandle(), gfx::EImageLayout::UNDEFINED, gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  api::Fence submit_fence (device);
  api::CommandList setup_list = gfx.CreateCommandList(GDM_HASH("SceneSetup"), gfx::ECommandListFlags::ONCE);
  setup_list.PushBarrier(barrier);

  DataStorage<api::RenderPass> render_passes {};
  render_passes.Create(GDM_HASH("MainRenderPass"), device);

  api::RenderPass& render_pass = render_passes.Get(GDM_HASH("MainRenderPass"));
  uint color_idx = 0;
  uint depth_idx = 1;
  uint input_idx = -1;
  render_pass.AddPassDescription(color_idx, gfx.GetSurfaceFormat(), gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);
  render_pass.AddPassDescription(depth_idx, depth_image.GetFormat<gfx::EFormatType>(), gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
  uint subpass_idx = 0;
  render_pass.CreateSubpass(gfx::EQueueType::GRAPHICS);
  render_pass.AddSubpassColorAttachments(subpass_idx, api::Attachments{color_idx});
  render_pass.AddSubpassDepthAttachments(subpass_idx, api::Attachment{depth_idx});
  render_pass.Finalize();

  DataStorage<api::Framebuffer> framebuffers {};
  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    api::ImageViews image_views {gfx.GetBackBufferViews()[i], &depth_image_view};
    framebuffers.Create(GDM_HASH_N("MainFB", i), gfx.GetDevice(), width, height, render_pass, image_views);
  }

  DataStorage<Shader> shaders {};
  shaders.Create(GDM_HASH("FlatVx"), "shaders/flat_vert.hlsl", gfx::EShaderType::VX);
  shaders.Create(GDM_HASH("FlatPx"), "shaders/flat_frag.hlsl", gfx::EShaderType::PX);

  ModelFactory::SetPath("../../_data/models/");
  MaterialFactory::SetPath("../../_data/materials/");
  TextureFactory::SetPath("../../_data/textures/");
  ImageFactory::SetPath("../../_models_new/textures/");

  ModelHandle cube_handle = ModelFactory::Load("cube_test.ply");
  AbstractModel* cube_model = ModelFactory::Get(cube_handle);
  AbstractMesh* cube_mesh = MeshFactory::Get(cube_model->meshes_.back());
  AbstractMaterial* cube_mat = MaterialFactory::Get(cube_model->materials_.back());
  AbstractTexture* cube_tex = TextureFactory::Get(cube_mat->diff_);
  AbstractImage* cube_pix = ImageFactory::Get(cube_tex->image_);

  // copy directly on gpu (depending on the hardware's architecture it may not be possible)
  const std::vector<float>& vx_data = cube_mesh->interleaving_vxs_buffer_;
  uint vx_buffer_size = static_cast<uint>(sizeof(float) * vx_data.size());
  auto vx_buffer = api::Buffer(&device, vx_buffer_size, gfx::VERTEX, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  vx_buffer.Map();
  vx_buffer.CopyDataToGpu(vx_data.data(), vx_data.size());
  vx_buffer.Unmap();

  // correct and fast way to copy data using staging buffers
  const std::vector<Vec3u>& indices_data = cube_mesh->faces_;
  uint indices_data_size = static_cast<uint>(sizeof(unsigned int) * 3 * indices_data.size());
  auto staging_buffer = api::Buffer(&device, indices_data_size, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  staging_buffer.Map();
  staging_buffer.CopyDataToGpu(indices_data.data(), indices_data.size());
  staging_buffer.Unmap();
  auto idx_buffer = api::Buffer(&device, indices_data_size, gfx::INDEX | gfx::TRANSFER_DST, gfx::DEVICE_LOCAL);
  setup_list.CopyBufferToBuffer(staging_buffer, idx_buffer, indices_data_size);

  // copy data using buffers instead of images as images is in optimal layout and copying possible only for linear layouts
  const auto& img_data = cube_pix->GetRaw();
  uint img_data_size = static_cast<uint>(img_data.size());
  auto staging_buffer2 = api::Buffer(&device, img_data_size, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  staging_buffer2.Map();
  staging_buffer2.CopyDataToGpu(img_data.data(), img_data.size());
  staging_buffer2.Unmap();
  gfx::ImageUsage cube_img_usage = gfx::TRANSFER_DST_IMG | gfx::SAMPLED;
  gfx::FormatType cube_img_format = gfx::UNORM4;
  uint cube_img_w = static_cast<uint>(cube_pix->GetWidth());
  uint cube_img_h = static_cast<uint>(cube_pix->GetHeight());
  auto cube_img = api::Image2D(&device, cube_img_w, cube_img_h, cube_img_usage, cube_img_format);
  auto cube_img_view = api::ImageView(device, cube_img, cube_img.GetFormat());
  auto barrier_undef_to_transfer = api::ImageBarrier(&device, cube_img, gfx::EImageLayout::UNDEFINED, gfx::EImageLayout::TRANSFER_DST_OPTIMAL);
  auto barrier_transfer_to_shader = api::ImageBarrier(&device, cube_img, gfx::EImageLayout::TRANSFER_DST_OPTIMAL, gfx::EImageLayout::SHADER_READ_OPTIMAL);
  setup_list.PushBarrier(barrier_undef_to_transfer);
  setup_list.CopyBufferToImage(staging_buffer2, cube_img, static_cast<uint>(img_data.size()));
  setup_list.PushBarrier(barrier_transfer_to_shader);

  api::Sampler cube_sampler(device, StdSamplerState{});

  std::vector<api::Buffer*> pocb_uniform;
  std::vector<api::Buffer*> pocb_staging;
  std::vector<api::BufferBarrier*> pocb_to_write_barriers;
  std::vector<api::BufferBarrier*> pocb_to_read_barriers;
  std::vector<api::Buffer*> pfcb_uniform;
  std::vector<api::Buffer*> pfcb_staging;
  std::vector<api::BufferBarrier*> pfcb_to_write_barriers;
  std::vector<api::BufferBarrier*> pfcb_to_read_barriers;

  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    auto* pocb_buf = GMNew api::Buffer(&device, sizeof(FlatVs_POCB), gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
    pocb_uniform.push_back(pocb_buf);
    auto* pocb_staging_buf = GMNew api::Buffer(&device, sizeof(FlatVs_POCB), gfx::TRANSFER_SRC, gfx::HOST_VISIBLE); // and COHERENT too?
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

  auto* descriptor_layout = GMNew api::DescriptorSetLayout(device);
  descriptor_layout->AddBinding(0, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  descriptor_layout->AddBinding(1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  descriptor_layout->AddBinding(2, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  descriptor_layout->AddBinding(3, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  descriptor_layout->Finalize();

  std::vector<api::DescriptorSet*> cube_descriptor_sets;

  for (uint i = 0; i < gfx.GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(device, *descriptor_layout, gfx.GetDescriptorPool());
    descriptor_set->UpdateContent(0, gfx::EResourceType::UNIFORM_BUFFER, *pfcb_uniform[i]);
    descriptor_set->UpdateContent(1, gfx::EResourceType::UNIFORM_BUFFER, *pocb_uniform[i]);
    descriptor_set->UpdateContent(2, gfx::EResourceType::SAMPLED_IMAGE, cube_img_view);
    descriptor_set->UpdateContent(3, gfx::EResourceType::SAMPLER, cube_sampler);
    descriptor_set->Finalize();
    cube_descriptor_sets.push_back(descriptor_set);
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

    scene::UpdateCamera(camera, input, dt);
    Mat4f view = camera.GetViewMx();
    Mat4f proj = camera.GetProjectionMx();
    pfcb.u_view_proj_ = proj * view;
    pocb.u_model_.SetCol(3, Vec3f(0.f, 1.f, 5.f));
    pocb.u_color_ = Vec4f(1.f, 1.f, 0.f, 1.f);

    cmd.PushBarrier(*present_to_write_barrier[curr_frame]);

    cmd.PushBarrier(*pfcb_to_write_barriers[curr_frame]);
    pfcb_staging[curr_frame]->Map();
    pfcb_staging[curr_frame]->CopyDataToGpu(&pfcb, 1);
    pfcb_staging[curr_frame]->Unmap();
    cmd.CopyBufferToBuffer(*pfcb_staging[curr_frame], *pfcb_uniform[curr_frame], sizeof(FlatVs_PFCB));
    cmd.PushBarrier(*pfcb_to_read_barriers[curr_frame]);

    cmd.PushBarrier(*pocb_to_write_barriers[curr_frame]);
    pocb_staging[curr_frame]->CopyDataToGpu(&pocb, 1);
    cmd.CopyBufferToBuffer(*pocb_staging[curr_frame], *pocb_uniform[curr_frame], sizeof(FlatVs_POCB));
    cmd.PushBarrier(*pocb_to_read_barriers[curr_frame]);
    
    auto& fb = framebuffers.Get(GDM_HASH_N("MainFB", curr_frame));
    cmd.BeginRenderPass(render_pass, fb, width, height);
    cmd.BindPipelineGraphics(pipeline);
    cmd.BindVertexBuffer(vx_buffer);
    cmd.BindIndexBuffer(idx_buffer);
    cmd.BindDescriptorSetGraphics({*cube_descriptor_sets[curr_frame]}, pipeline);
    cmd.DrawIndexed(indices_data);
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