// *************************************************************
// File:    gbuffer_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gbuffer_pass.h"

// --public

void gdm::GbufferPass::CreateImages(api::CommandList& cmd)
{
  const int total_images = 4;
  const int rw_images = 3;

  data_.images_.resize(total_images);
  data_.image_views_.resize(total_images);
  data_.image_barriers_to_read_.resize(rw_images);
  data_.image_barriers_to_write_.resize(rw_images);

  for(int i = 0; i < rw_images; ++i)
  {
    data_.images_[i] = GMNew api::Image2D(device_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
    data_.images_[i]->GetProps()
      .AddFormatType(gfx::EFormatType::F4HALF)
      .AddImageUsage(gfx::EImageUsage::COLOR_ATTACHMENT | gfx::EImageUsage::SAMPLED)
      .Create();
    data_.image_views_[i] = GMNew api::ImageView(*device_);
    data_.image_views_[i]->GetProps()
      .AddImage(*data_.images_[i])
      .AddFormatType(data_.images_[i]->GetFormat())
      .Create();
    data_.image_barriers_to_read_[i] = GMNew api::ImageBarrier;
    data_.image_barriers_to_read_[i]->GetProps()
      .AddImage(*data_.images_[i])
      .AddOldLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
      .Finalize();
    data_.image_barriers_to_write_[i] = GMNew api::ImageBarrier;
    data_.image_barriers_to_write_[i]->GetProps()
      .AddImage(*data_.images_[i])
      .AddOldLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
      .AddNewLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .Finalize();
    api::ImageBarrier init_barrier;
    init_barrier.GetProps()
      .AddImage(*data_.images_[i])
      .AddOldLayout(gfx::EImageLayout::UNDEFINED)
      .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
      .Finalize();
    cmd.PushBarrier(init_barrier);
  }

  api::Image2D* gbuffer_depth = GMNew api::Image2D(device_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
  data_.images_[3] = gbuffer_depth;
  api::ImageView* gbuffer_depth_view = GMNew api::ImageView(*device_);
  data_.image_views_[3] = gbuffer_depth_view;
  
  data_.images_[3]->GetProps()
    .AddFormatType(gfx::EFormatType::D16_UNORM)
    .AddImageUsage(gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT)
    .Create();
  data_.image_views_[3]->GetProps()
    .AddImage(*data_.images_.back())
    .AddFormatType(data_.images_.back()->GetFormat())
    .Create();

  api::ImageBarrier depth_barrier;
  depth_barrier.GetProps()
    .AddImage(*data_.images_[3])
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .Finalize();
  cmd.PushBarrier(depth_barrier);
}

void gdm::GbufferPass::CreateFramebuffer()
{
  uint height = rdr_->GetSurfaceHeight();
  uint width = rdr_->GetSurfaceWidth();
  data_.fb_ = GMNew api::Framebuffer(*device_, width, height, *pass_, data_.image_views_);
}

void gdm::GbufferPass::CreateRenderPass()
{
  pass_ = GMNew api::RenderPass(*device_);

  for (int i = 0; i < 3; ++i)
  {
    pass_->AddAttachmentDescription(i)
      .AddFormat(gfx::EFormatType::F4HALF)
      .AddInitLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddFinalLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddRefLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);
  }
  
  uint gbuff_depth_idx = 3;
  pass_->AddAttachmentDescription(gbuff_depth_idx)
    .AddFormat(gfx::EFormatType::D16_UNORM)
    .AddInitLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .AddFinalLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .AddRefLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  uint subpass_idx = pass_->CreateSubpass(gfx::EQueueType::GRAPHICS);
  pass_->AddSubpassColorAttachments(subpass_idx, api::Attachments{0,1,2});
  pass_->AddSubpassDepthAttachments(subpass_idx, api::Attachment{3});
  pass_->Finalize();
}

void gdm::GbufferPass::CreateDescriptorSet(api::ImageViews& diffuse, api::ImageViews& specular, api::ImageViews& normals)
{
  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(1, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(2, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(3, static_cast<uint>(diffuse.size()), gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::EBindingFlags::VARIABLE_DESCRIPTOR);
  // todo: normals and pe pass in 3
  // dsl->AddBinding(4, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::EBindingFlags::VARIABLE_DESCRIPTOR);
  // dsl->AddBinding(5, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::EBindingFlags::VARIABLE_DESCRIPTOR);
  dsl->Finalize();

  data_.sampler_ = GMNew api::Sampler(*device_, StdSamplerState{});

  auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_.pfcb_uniform_);
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, *data_.pocb_uniform_);
  descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(2, *data_.sampler_);
  descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(3, diffuse);
  descriptor_set->Finalize();
  
  data_.descriptor_set_layout_ = dsl;
  data_.descriptor_set_ = descriptor_set;
}

void gdm::GbufferPass::CreatePipeline()
{
  Shader vx_shader("shaders/gbuffer_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("shaders/gbuffer_frag.hlsl", gfx::EShaderType::PX);

  float width = static_cast<float>(rdr_->GetSurfaceWidth());
  float height = static_cast<float>(rdr_->GetSurfaceHeight());
  ViewportDesc vp{0, height, width, -height, 0, 1};

  pipeline_ = GMNew api::Pipeline(*device_);
  pipeline_->SetShaderStage(vx_shader, gfx::EShaderType::VX);
  pipeline_->SetShaderStage(px_shader, gfx::EShaderType::PX);
  pipeline_->SetViewportState(vp);
  pipeline_->SetRasterizerState(StdRasterizerDesc{});
  pipeline_->SetInputLayout(StdInputLayout{});
  pipeline_->SetRenderPass(*pass_);
  pipeline_->SetDescriptorSetLayouts(api::DescriptorSetLayouts{*data_.descriptor_set_layout_});
  const int depth_attachments_cnt = 1;
  pipeline_->SetBlendAttachmentsCount(pass_->GetPassAttachmentsCount() - depth_attachments_cnt);
  pipeline_->Compile();
}