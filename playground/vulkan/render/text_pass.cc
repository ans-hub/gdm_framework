// *************************************************************
// File:    text_pass.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "text_pass.h"

#include <vector>

#include "render/api.h"
#include "render/defines.h"
#include "render/debug.h"
#include "system/diff_utils.h"
#include "system/event_point.h"

// --public different

void gdm::TextPass::BindFont(const Font* font, const api::ImageView* font_texture)
{
  font_ = font;
  font_texture_ = font_texture;
}

void gdm::TextPass::BindFramebuffer(api::Framebuffer* fb, uint frame_num)
{
  data_[frame_num].fb_ = fb;
}

// --public create

gdm::TextPassData::TextPassData(api::Renderer& rdr)
  : rdr_{&rdr}
  , device_{device_}
{ }

gdm::TextPass::TextPass(int frame_count, api::Renderer& rdr)
  : rdr_{&rdr}
  , device_{&rdr.GetDevice()}
  , data_(frame_count, rdr)
  , font_{nullptr}
  , font_texture_{nullptr}
  , strings_{}
{ }

gdm::TextPass::~TextPass()
{
  Cleanup();
}

void gdm::TextPass::Cleanup()
{
  for (auto&& data : data_)
  {
    GMDelete(data.pocb_uniform_fs_);
    GMDelete(data.fb_);
    GMDelete(data.vertex_buffer_);
    GMDelete(data.descriptor_set_);
    GMDelete(data.present_to_read_barrier_);
    GMDelete(data.present_to_write_barrier_);
  }

  GMDelete(sampler_);
  GMDelete(pass_);
  GMDelete(pipeline_);
}

void gdm::TextPass::CreateUniforms(api::CommandList& cmd, uint frame_num)
{
  data_[frame_num].pocb_uniform_fs_ = GMNew api::Buffer(device_, sizeof(TextFs_POCB) * 1, gfx::UNIFORM, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
  [[maybe_unused]] const bool map_and_never_unmap = [&](){ data_[frame_num].pocb_uniform_fs_->Map(); return true; }();
}

void gdm::TextPass::CreateVertexBuffer(api::CommandList& cmd, uint frame_num)
{
  const int quad_size = 4;
  const int buffer_size = v_max_string_ * sizeof(Vec4f) * quad_size;

  data_[frame_num].vertex_buffer_ = GMNew api::Buffer(device_, uint(buffer_size), gfx::VERTEX, gfx::HOST_VISIBLE | gfx::HOST_COHERENT);
}

void gdm::TextPass::CreateBarriers(api::CommandList& cmd)
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
    .AddStoreOp(gfx::EAttachmentStoreOp::STORE_OP);

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

  sampler_ = GMNew api::Sampler(*device_, TextSamplerDesc{});

  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  uint pfcb = dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::FRAGMENT_STAGE);
  uint sampler = dsl->AddBinding(1, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_pos = dsl->AddBinding(2, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  
  dsl->Finalize();

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_[i].pocb_uniform_fs_);
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

  api::BlendState* blend_state = GMNew api::BlendState(*device_);

  blend_state->AddAttachmentDescription(0)
    .SetEnabled(true)
    .SetColorWriteMask(gfx::R | gfx::G | gfx::B | gfx::A)
    .SetSrcColorBlendFactor(gfx::EBlendFactor::SRC_ALPHA)
    .SetDstColorBlendFactor(gfx::EBlendFactor::ONE_MINUS_SRC_ALPHA)
    .SetSrcAlphaBlendFactor(gfx::EBlendFactor::SRC_ALPHA)
    .SetDstAlphaBlendFactor(gfx::EBlendFactor::ONE_MINUS_SRC_ALPHA)
    .SetColorBlendOp(gfx::EBlendOp::ADD)
    .SetAlphaBlendOp(gfx::EBlendOp::ADD);
  blend_state->Finalize();

  pipeline_->SetBlendState(*blend_state);
  pipeline_->Compile();

  GMDelete(dsl);
}

// --public update

void gdm::TextPass::UpdateVertexData(api::CommandList& cmd, uint curr_frame, const std::vector<TextData>& text_data)
{
  GDM_EVENT_POINT("TextMap", GDM_LABEL_S(color::LightGray));

  ASSERTF(font_, "Font is not binded");

  std::vector<Vec4f> mapped_data;
  mapped_data.reserve(text_data.size() * v_vxs_per_char_);

  const float w = (float)rdr_->GetSurfaceWidth();
	const float h = (float)rdr_->GetSurfaceHeight();
  const float ar = h/w;
  const float prop = 1.f / w * 2.f;
  const float font_height = font_->GetMetrics().font_height_;

  for (auto&& data : text_data)
  {
    float x = data.pos_.x;
    float y = data.pos_.y + font_height;

    for (auto&& letter : data.text_)
    {
      const Font::Character& char_data = (*font_)[static_cast<int>(letter)];

      const float x0 = ((x + char_data.coords_.x0) * prop * ar) - 1.f;
      const float x1 = ((x + char_data.coords_.x1) * prop * ar) - 1.f;
      const float y1 = ((y + char_data.coords_.y0) * prop) - 1.f;
      const float y0 = ((y + char_data.coords_.y1) * prop) - 1.f;
      const float u0 = char_data.uv_.u0;
      const float u1 = char_data.uv_.u1;
      const float v0 = char_data.uv_.v0;
      const float v1 = char_data.uv_.v1;

      mapped_data.push_back( Vec4f{ x0, y0, u0, v0 } );
      mapped_data.push_back( Vec4f{ x1, y0, u1, v0 } );
      mapped_data.push_back( Vec4f{ x0, y1, u0, v1 } );
      mapped_data.push_back( Vec4f{ x1, y1, u1, v1 } );

      x += char_data.advance_ + 1;
    }

    strings_.push_back(std::make_pair(uint(data.text_.size()), data.color_));
  }
  
  auto& vbuf = *data_[curr_frame].vertex_buffer_;
  
  vbuf.Map();
  vbuf.CopyDataToGpu(mapped_data.data(), 0, mapped_data.size());
  vbuf.Unmap();
}

void gdm::TextPass::Draw(api::CommandList& cmd, uint curr_frame)
{
  auto& data = data_[curr_frame];

  if (strings_.empty())
    return;

  GDM_EVENT_POINT("TextPass", GDM_LABEL_S(color::LightYellow));
  
  cmd.PushBarrier(*data.present_to_write_barrier_);
  
  api::DescriptorSets descriptor_sets {*data_[curr_frame].descriptor_set_};

  cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_, gfx::Offsets{});      
  cmd.BindPipelineGraphics(*pipeline_);
  
  uint characters_offset = 0;

  cmd.BeginRenderPass(*pass_, *data.fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
  cmd.BindVertexBuffer(*data.vertex_buffer_);

  for (auto&& [characters_count, color] : strings_)
  {
    GDM_EVENT_POINT("DrawString", GDM_LABEL_I(color::LightGray));

    TextFs_POCB ub {color};
    data_[curr_frame].pocb_uniform_fs_->CopyDataToGpu(&ub, 0, 1);

    for (uint i = characters_offset; i < characters_count + characters_offset; ++i)
      cmd.Draw(v_vxs_per_char_, i * v_vxs_per_char_);
     
    characters_offset += characters_count;
  }

  cmd.EndRenderPass();
  cmd.PushBarrier(*data_[curr_frame].present_to_read_barrier_);

  strings_.clear();
}
