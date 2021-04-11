// *************************************************************
// File:    debug_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "debug_pass.h"

#include "render/api.h"
#include "render/defines.h"
#include "render/debug.h"
#include "render/colors.h"
#include "system/diff_utils.h"
#include "system/event_point.h"
#include "data/model_factory.h"
#include "scene/gui_draw.h"

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_win32.h"

// --public create

gdm::DebugPass::DebugPass(int frame_count, api::Renderer& rdr)
  : rdr_{&rdr}
  , device_{&rdr.GetDevice()}
  , data_(frame_count, rdr)
  , gui_draw_callbacks_{}
{ }

void gdm::DebugPass::CreateUniforms(api::CommandList& cmd, uint frame_num)
{
  data_[frame_num].pfcb_staging_vs_ = GMNew api::Buffer(device_, sizeof(DebugVs_PFCB) * 1, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE);
  data_[frame_num].pfcb_uniform_vs_ = GMNew api::Buffer(device_, sizeof(DebugVs_PFCB) * 1, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  data_[frame_num].pfcb_to_read_barrier_ = GMNew api::BufferBarrier(device_, *data_[frame_num].pfcb_uniform_vs_, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  data_[frame_num].pfcb_to_write_barrier_ = GMNew api::BufferBarrier(device_, *data_[frame_num].pfcb_uniform_vs_, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  cmd.PushBarrier(*data_[frame_num].pfcb_to_read_barrier_);
}

void gdm::DebugPass::CreateVertexBuffer(api::CommandList& cmd, uint frame_num, uint64 buffer_size)
{
  data_[frame_num].vertex_buffer_ = GMNew api::Buffer(device_, uint(buffer_size), gfx::VERTEX, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
}

void gdm::DebugPass::CreateBarriers(api::CommandList& cmd)
{
  auto present_images = rdr_->GetBackBufferImages();

  for(auto&& [i, data] : Enumerate(data_))
  {
    data.present_to_read_barrier_ = GMNew api::ImageBarrier();
    data.present_to_write_barrier_ = GMNew api::ImageBarrier();

    data.present_to_read_barrier_->GetProps()
      .AddImage(present_images[i])
      .AddOldLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddNewLayout(gfx::EImageLayout::PRESENT_SRC)
      .Finalize();

    data.present_to_write_barrier_->GetProps()
      .AddImage(present_images[i])
      .AddOldLayout(gfx::EImageLayout::PRESENT_SRC)
      .AddNewLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .Finalize();
  }
}

void gdm::DebugPass::CreateFramebuffer()
{
  uint height = rdr_->GetSurfaceHeight();
  uint width = rdr_->GetSurfaceWidth();

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    api::ImageViews image_views { rdr_->GetBackBufferViews()[i] };
    data_[i].fb_ = GMNew api::Framebuffer(*device_, width, height, *pass_, image_views);
  }
}

void gdm::DebugPass::CreateRenderPass()
{
  pass_ = GMNew api::RenderPass(*device_);

  uint color_idx = 0;

  pass_->AddAttachmentDescription(color_idx)
    .AddFormat(rdr_->GetSurfaceFormat())
    .AddInitLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddFinalLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddRefLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddLoadOp(gfx::EAttachmentLoadOp::LOAD_OP)
    .AddStoreOp(gfx::EAttachmentStoreOp::STORE_OP);

  uint subpass_idx = pass_->CreateSubpass(gfx::EQueueType::GRAPHICS);
  pass_->AddSubpassColorAttachments(subpass_idx, api::Attachments{color_idx});
  pass_->Finalize();
}

void gdm::DebugPass::CreatePipeline()
{
  Shader vx_shader("assets/shaders/debug_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("assets/shaders/debug_frag.hlsl", gfx::EShaderType::PX);

  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  uint pfcb = dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  dsl->Finalize();

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_[i].pfcb_uniform_vs_);
    descriptor_set->Finalize();
    data_[i].descriptor_set_ = descriptor_set;
  }

  float width = static_cast<float>(rdr_->GetSurfaceWidth());
  float height = static_cast<float>(rdr_->GetSurfaceHeight());
  
  ViewportDesc vp{0, height, width, -height, 0, 1};

  pipeline_ = GMNew api::Pipeline(*device_);
  pipeline_->SetShaderStage(vx_shader, gfx::EShaderType::VX);
  pipeline_->SetShaderStage(px_shader, gfx::EShaderType::PX);
  pipeline_->SetViewportState(vp);
  pipeline_->SetRasterizerState(WireRasterizerDesc{});
  pipeline_->SetInputLayout(WireInputLayout{});
  pipeline_->SetDescriptorSetLayouts(api::DescriptorSetLayouts{*dsl});
  pipeline_->SetRenderPass(*pass_);
  
  api::BlendState* blend_state = GMNew api::BlendState(*device_);
  blend_state->AddAttachmentDescription(0)
    .SetEnabled(false)
    .SetColorWriteMask(gfx::R | gfx::G | gfx::B | gfx::A);
  blend_state->Finalize();

  pipeline_->SetBlendState(*blend_state);
  pipeline_->Compile();
}

void gdm::DebugPass::CreateGui()
{
  IMGUI_CHECKVERSION();
  
  ASSERT(ImGui::GetCurrentContext() == nullptr);
 
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
 
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  
  ImGui_ImplWin32_Init(rdr_->GetSurfaceWindow());
  io.DisplaySize.x = rdr_->GetSurfaceWidth();
  io.DisplaySize.y = rdr_->GetSurfaceHeight();

  auto check_vk_result = [](VkResult res) { ASSERT(res == VK_SUCCESS); };

  ImGui_ImplVulkan_InitInfo init_info = {};

  init_info.Instance = rdr_->GetInstance();
  init_info.PhysicalDevice = rdr_->GetDevice().GetPhysicalDevice().info_.device_;
  init_info.Device = rdr_->GetDevice();
  init_info.QueueFamily = rdr_->GetDevice().GetPhysicalDevice().queue_id;
  init_info.Queue = rdr_->GetDevice().GetQueue(gfx::EQueueType::GRAPHICS);
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = rdr_->GetDescriptorPool();
  init_info.Allocator = VK_NULL_HANDLE;
  init_info.MinImageCount = rdr_->GetBackBuffersCount();
  init_info.ImageCount = init_info.MinImageCount;
  init_info.CheckVkResultFn = check_vk_result;
  ImGui_ImplVulkan_Init(&init_info, *pass_);

  auto command_buffer = rdr_->CreateCommandList(GDM_HASH("ImGuiSetup"), gfx::ECommandListFlags::ONCE);
  ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
  command_buffer.Finalize();
  rdr_->SubmitCommandLists(api::CommandLists{command_buffer}, api::Semaphores::empty, api::Semaphores::empty, api::Fence::null);
  rdr_->WaitForGpuIdle();
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

// --public update

void gdm::DebugPass::UpdateUniformsData(uint curr_frame, const CameraEul& camera)
{
  Mat4f view = camera.GetViewMx();
  Mat4f proj = camera.GetProjectionMx();
  data_[curr_frame].pfcb_data_vs_.view_proj_ = proj * view;
}

void gdm::DebugPass::UpdateUniforms(api::CommandList& cmd, uint frame_num)
{
  GDM_EVENT_POINT("DebugUniforms", GDM_LABEL_S(color::LightGreen));

  cmd.PushBarrier(*data_[frame_num].pfcb_to_write_barrier_);
  data_[frame_num].pfcb_staging_vs_->Map();
  data_[frame_num].pfcb_staging_vs_->CopyDataToGpu(&data_[frame_num].pfcb_data_vs_, 0, 1);
  data_[frame_num].pfcb_staging_vs_->Unmap();
  cmd.CopyBufferToBuffer(*data_[frame_num].pfcb_staging_vs_, *data_[frame_num].pfcb_uniform_vs_, sizeof(DebugVs_PFCB));
  cmd.PushBarrier(*data_[frame_num].pfcb_to_read_barrier_);
}

void gdm::DebugPass::UpdateVertexData(api::CommandList& cmd, uint curr_frame, const std::vector<DebugData>& debug_data)
{
  auto& vbuf = *data_[curr_frame].vertex_buffer_;

  vbuf.Map();
  vbuf.CopyDataToGpu(debug_data.data(), 0, debug_data.size());
  vbuf.Unmap();

  data_[curr_frame].vertices_count_ = debug_data.size();
}

void gdm::DebugPass::Draw(api::CommandList& cmd, uint curr_frame, bool debug_stage_active, bool gui_stage_active)
{
  GDM_EVENT_POINT("DebugPass", GDM_LABEL_S(color::LightGreen));
 
  auto& data = data_[curr_frame];

  api::DescriptorSets descriptor_sets {*data_[curr_frame].descriptor_set_};

  cmd.PushBarrier(*data.present_to_write_barrier_);
  cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_, gfx::Offsets{});      
  cmd.BindPipelineGraphics(*pipeline_);
  cmd.BeginRenderPass(*pass_, *data.fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
  
  if (debug_stage_active && data.vertices_count_ != 0)
  {
    GDM_EVENT_POINT("DebugPass (prim)", GDM_LABEL_S(color::LightGreen));

    cmd.BindVertexBuffer(*data.vertex_buffer_);
    cmd.Draw(data.vertices_count_);
  }

  data.vertices_count_ = 0;

  if (gui_stage_active)
  {
    GDM_EVENT_POINT("DebugPass (gui)", GDM_LABEL_S(color::LightGreen));

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    for (auto&& cb : gui_draw_callbacks_)
      cb();

    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

    if (!is_minimized)
      ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);  
  }

  cmd.EndRenderPass();
  cmd.PushBarrier(*data_[curr_frame].present_to_read_barrier_);
}
