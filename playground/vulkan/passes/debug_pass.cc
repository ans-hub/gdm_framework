// *************************************************************
// File:    debug_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "debug_pass.h"

#include "render/api.h"
#include "render/defines.h"
#include "system/diff_utils.h"
#include "data/model_factory.h"

// --public create

void gdm::DebugPass::CreateUniforms(api::CommandList& cmd, uint frame_num)
{
  data_[frame_num].pfcb_staging_vs_ = GMNew api::Buffer(device_, sizeof(DebugVs_PFCB) * 1, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE);
  data_[frame_num].pfcb_uniform_vs_ = GMNew api::Buffer(device_, sizeof(DebugVs_PFCB) * 1, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  data_[frame_num].pfcb_to_read_barrier_ = GMNew api::BufferBarrier(device_, *data_[frame_num].pfcb_uniform_vs_, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  data_[frame_num].pfcb_to_write_barrier_ = GMNew api::BufferBarrier(device_, *data_[frame_num].pfcb_uniform_vs_, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  cmd.PushBarrier(*data_[frame_num].pfcb_to_read_barrier_);
}

void gdm::DebugPass::CreateBuffer(api::CommandList& cmd, uint frame_num, uint64 buffer_size)
{
  data_[frame_num].vertex_buffer_ = GMNew api::Buffer(device_, uint(buffer_size), gfx::VERTEX, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
}

void gdm::DebugPass::CreateImages(api::CommandList& cmd)
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
    .AddStoreOp(gfx::EAttachmentStoreOp::STORE_DONT_CARE);

  uint subpass_idx = pass_->CreateSubpass(gfx::EQueueType::GRAPHICS);
  pass_->AddSubpassColorAttachments(subpass_idx, api::Attachments{color_idx});
  pass_->Finalize();
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
  pipeline_->Compile();
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
  cmd.PushBarrier(*data_[frame_num].pfcb_to_write_barrier_);
  data_[frame_num].pfcb_staging_vs_->Map();
  data_[frame_num].pfcb_staging_vs_->CopyDataToGpu(&data_[frame_num].pfcb_data_vs_, 1);
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

void gdm::DebugPass::Draw(api::CommandList& cmd, uint curr_frame)
{
  auto& data = data_[curr_frame];

  if (data.vertices_count_ == 0)
    return;

  cmd.PushBarrier(*data.present_to_write_barrier_);
  
  api::DescriptorSets descriptor_sets {*data_[curr_frame].descriptor_set_};

  cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_, gfx::Offsets{});      
  cmd.BindPipelineGraphics(*pipeline_);
  cmd.BeginRenderPass(*pass_, *data.fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
  cmd.BindVertexBuffer(*data.vertex_buffer_);
  cmd.Draw(data.vertices_count_);
  cmd.EndRenderPass();

  cmd.PushBarrier(*data_[curr_frame].present_to_read_barrier_);
}