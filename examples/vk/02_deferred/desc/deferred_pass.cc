// *************************************************************
// File:    deferred_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "deferred_pass.h"

#include "system/diff_utils.h"

// --public

void gdm::DeferredPass::CreateImages(api::CommandList& cmd)
{
  auto present_images = rdr_->GetBackBufferImages();

  for(auto& [i,data] : Enumerate(data_))
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

    data.depth_image_ = GMNew api::Image2D(device_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());
    data.depth_image_view_ = GMNew api::ImageView(*device_);

    data.depth_image_->GetProps()
      .AddImageUsage(gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT)
      .AddFormatType(gfx::EFormatType::D16_UNORM)
      .Create();
    data.depth_image_view_->GetProps()
      .AddImage(data.depth_image_->GetHandle())
      .AddFormatType(data.depth_image_->GetFormat())
      .Create();

    api::ImageBarrier depth_barrier;
    depth_barrier.GetProps()
      .AddImage(*data.depth_image_)
      .AddOldLayout(gfx::EImageLayout::UNDEFINED)
      .AddNewLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
      .Finalize();
    cmd.PushBarrier(depth_barrier);
  }
}

void gdm::DeferredPass::CreateFramebuffer()
{
  uint height = rdr_->GetSurfaceHeight();
  uint width = rdr_->GetSurfaceWidth();

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    api::ImageViews image_views { rdr_->GetBackBufferViews()[i], data_[i].depth_image_view_};
    data_[i].fb_ = GMNew api::Framebuffer(*device_, width, height, *pass_, image_views);
  }
}

void gdm::DeferredPass::CreateRenderPass()
{
  pass_ = GMNew api::RenderPass(*device_);

  uint color_idx = 0;
  pass_->AddAttachmentDescription(color_idx)
    .AddFormat(rdr_->GetSurfaceFormat())
    .AddInitLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddFinalLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddRefLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);
  
  uint depth_idx = 1;
  pass_->AddAttachmentDescription(depth_idx)
    .AddFormat(data_[0].depth_image_->GetFormat<gfx::EFormatType>())  // TODO: make single depth image
    .AddInitLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .AddFinalLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .AddRefLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  uint subpass_idx = pass_->CreateSubpass(gfx::EQueueType::GRAPHICS);
  pass_->AddSubpassColorAttachments(subpass_idx, api::Attachments{color_idx});
  pass_->AddSubpassDepthAttachments(subpass_idx, api::Attachment{depth_idx});
  pass_->Finalize();
}

void gdm::DeferredPass::CreatePipeline(const api::ImageViews& gbuffer_image_views)
{
  Shader vx_shader("shaders/deferred_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("shaders/deferred_frag.hlsl", gfx::EShaderType::PX);

  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  uint pfcb_lights = dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::FRAGMENT_STAGE);
  uint sampler     = dsl->AddBinding(1, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_pos   = dsl->AddBinding(2, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_diff  = dsl->AddBinding(3, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_norm  = dsl->AddBinding(4, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->Finalize();

  sampler_ = GMNew api::Sampler(*device_, StdSamplerState{});

  for (uint i = 0; i < rdr_->GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(0, *data_[i].pfcb_uniform_);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(1, *sampler_);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(2, *gbuffer_image_views[0]);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(3, *gbuffer_image_views[1]);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(4, *gbuffer_image_views[2]);
    descriptor_set->Finalize();
    data_[i].descriptor_set_ = descriptor_set;
  }

  float width = static_cast<float>(rdr_->GetSurfaceWidth());
  float height = static_cast<float>(rdr_->GetSurfaceHeight());
  ViewportDesc vp{0, 0, width, height, 0, 1};

  pipeline_ = GMNew api::Pipeline(*device_);
  pipeline_->SetShaderStage(vx_shader, gfx::EShaderType::VX);
  pipeline_->SetShaderStage(px_shader, gfx::EShaderType::PX);
  pipeline_->SetViewportState(vp);
  pipeline_->SetRasterizerState(ReverseRasterizerDesc{});
  pipeline_->SetInputLayout(EmptyInputLayout{});
  pipeline_->SetRenderPass(*pass_);
  pipeline_->SetDescriptorSetLayouts(api::DescriptorSetLayouts{*dsl});
  pipeline_->Compile();
}