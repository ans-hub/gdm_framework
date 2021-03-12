// *************************************************************
// File:    text_pass.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "text_pass.h"

#include <vector>

#include "render/api.h"
#include "render/defines.h"
#include "system/diff_utils.h"

// --public different

void gdm::TextPass::BindFont(const Font* font, api::ImageView* font_texture)
{
  font_ = font;
  font_texture_ = font_texture;
}

void gdm::TextPass::BindFramebuffer(api::Framebuffer* fb, uint frame_num)
{
  data_[frame_num].fb_ = fb;
}

// --public create

void gdm::TextPass::CreateUniforms(api::CommandList& cmd, uint frame_num)
{
  data_[frame_num].pfcb_uniform_vs_ = GMNew api::Buffer(device_, sizeof(TextVs_PFCB) * 1, gfx::UNIFORM, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  [[maybe_unused]] const bool map_and_never_unmap = [&](){ data_[frame_num].pfcb_uniform_vs_->Map(); return true; }();
}

void gdm::TextPass::CreateVertexBuffer(api::CommandList& cmd, uint frame_num)
{
  const int buffer_size = v_max_string_;
  data_[frame_num].vertex_buffer_ = GMNew api::Buffer(device_, uint(buffer_size), gfx::VERTEX, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
}

void gdm::TextPass::CreateRenderPass()
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

void gdm::TextPass::CreateFramebuffer()
{
  uint height = rdr_->GetSurfaceHeight();
  uint width = rdr_->GetSurfaceWidth();

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    api::ImageViews image_views { rdr_->GetBackBufferViews()[i] };
    data_[i].fb_ = GMNew api::Framebuffer(*device_, width, height, *pass_, image_views);
  }
}

void gdm::TextPass::CreatePipeline()
{
  Shader vx_shader("assets/shaders/text_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("assets/shaders/text_frag.hlsl", gfx::EShaderType::PX);

  sampler_ = GMNew api::Sampler(*device_, StdSamplerDesc{});

  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  uint pfcb = dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  uint sampler = dsl->AddBinding(1, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_pos = dsl->AddBinding(2, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  
  dsl->Finalize();

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_[i].pfcb_uniform_vs_);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(1, *sampler_);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(2, *font_texture_);
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
  pipeline_->SetRasterizerState(StdRasterizerDesc{});
  pipeline_->SetInputLayout(TextInputLayout{});
  pipeline_->SetDescriptorSetLayouts(api::DescriptorSetLayouts{*dsl});
  pipeline_->SetRenderPass(*pass_);
  pipeline_->Compile();
}

// --public update

// begin

void gdm::TextPass::UpdateUniformsData(uint curr_frame, const CameraEul& camera)
{
  Mat4f view = camera.GetViewMx();
  Mat4f proj = camera.GetProjectionMx();
  data_[curr_frame].pfcb_data_vs_.view_proj_ = proj * view;
}

void gdm::TextPass::UpdateUniforms(api::CommandList& cmd, uint frame_num)
{
  data_[frame_num].pfcb_uniform_vs_->CopyDataToGpu(&data_[frame_num].pfcb_data_vs_, 0, 1);
}

void gdm::TextPass::UpdateVertexData(api::CommandList& cmd, uint curr_frame, const std::vector<TextData>& text_data)
{
  ASSERTF(font_, "Font is not binded");

  auto& vbuf = *data_[curr_frame].vertex_buffer_;

  vbuf.Map();

  std::vector<Vec4f> mapped_data;
  mapped_data.reserve(text_data.size() * v_vxs_per_char_);
 
  for (auto&& data : text_data)
  {
    for (auto&& letter : data.text_)
    {
      const Font::Character& char_data = (*font_)[static_cast<int>(letter)];
      
      const float x = data.pos_.x;
      const float y = data.pos_.y;
      const float x0 = float(char_data.coords_.x0);
      const float x1 = float(char_data.coords_.x1);
      const float y0 = float(char_data.coords_.y0);
      const float y1 = float(char_data.coords_.y1);

      mapped_data.push_back( Vec4f{ x + x0, y + y0, x0, y0 } );
      mapped_data.push_back( Vec4f{ x + x1, y + y0, x1, y0 } );
      mapped_data.push_back( Vec4f{ x + x0, y + y1, x0, y1 } );
      mapped_data.push_back( Vec4f{ x + x1, y + y1, x1, y1 } );
    }
  }
  
  vbuf.CopyDataToGpu(mapped_data.data(), 0, mapped_data.size());
  vbuf.Unmap();

  characters_count_ = mapped_data.size();
}

void gdm::TextPass::Draw(api::CommandList& cmd, uint curr_frame)
{
  auto& data = data_[curr_frame];

  if (characters_count_ == 0)
    return;

  data.vertex_buffer_->Map();

  cmd.PushBarrier(*data.present_to_write_barrier_);
  
  api::DescriptorSets descriptor_sets {*data_[curr_frame].descriptor_set_};

  cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_, gfx::Offsets{});      
  cmd.BindPipelineGraphics(*pipeline_);
  cmd.BeginRenderPass(*pass_, *data.fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
  cmd.BindVertexBuffer(*data.vertex_buffer_);

  for (uint i = 0; i < characters_count_; ++i)
    cmd.Draw(v_vxs_per_char_, i * v_vxs_per_char_);

  cmd.EndRenderPass();

  cmd.PushBarrier(*data_[curr_frame].present_to_read_barrier_);

  data.vertex_buffer_->Unmap();
  characters_count_ = 0;
}