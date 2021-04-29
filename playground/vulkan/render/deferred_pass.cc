// *************************************************************
// File:    deferred_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "deferred_pass.h"

#include "render/debug.h"
#include "system/diff_utils.h"
#include "system/event_point.h"

//--public

gdm::DeferredPass::DeferredPass(int frame_count, api::Renderer& ctx)
  : ctx_{&ctx}
  , device_{&ctx.GetDevice()}
  , data_(frame_count, DeferredPassData{})
{ }

gdm::DeferredPass::~DeferredPass()
{
  CleanupInternals();
  CleanupPipeline();
}

void gdm::DeferredPass::CleanupInternals()
{
  for (auto&& data : data_)
  {
    GMDelete(data.pfcb_staging_ps_);
    GMDelete(data.pfcb_uniform_ps_);
    GMDelete(data.pfcb_to_write_barrier_);
    GMDelete(data.pfcb_to_read_barrier_);
  }
}

void gdm::DeferredPass::CleanupPipeline()
{
  for (auto&& data : data_)
  {
    GMDelete(data.descriptor_set_);
    GMDelete(data.depth_image_);
    GMDelete(data.depth_image_view_);
    GMDelete(data.present_to_read_barrier_);
    GMDelete(data.present_to_write_barrier_);
    GMDelete(data.fb_);
  }
  GMDelete(sampler_);
  GMDelete(pass_);
  GMDelete(pipeline_);
}

void gdm::DeferredPass::CreateUniforms(api::CommandList& cmd, uint frame_num)
{
  data_[frame_num].pfcb_staging_ps_ = api::Resource<api::Buffer>(device_, sizeof(DeferredPs_PFCB) * 1)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE);
  
  data_[frame_num].pfcb_uniform_ps_ = api::Resource<api::Buffer>(device_, sizeof(DeferredPs_PFCB) * 1)
    .AddUsage(gfx::TRANSFER_DST | gfx::UNIFORM)
    .AddMemoryType(gfx::DEVICE_LOCAL);

  data_[frame_num].pfcb_to_read_barrier_ = api::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_[frame_num].pfcb_uniform_ps_)
    .AddOldAccess(gfx::EAccess::TRANSFER_WRITE)
    .AddNewAccess(gfx::EAccess::UNIFORM_READ);
  
  data_[frame_num].pfcb_to_write_barrier_ = api::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_[frame_num].pfcb_uniform_ps_)
    .AddOldAccess(gfx::EAccess::UNIFORM_READ)
    .AddNewAccess(gfx::EAccess::TRANSFER_WRITE);

  cmd.PushBarrier(*data_[frame_num].pfcb_to_read_barrier_);
}

void gdm::DeferredPass::CreateImages(api::CommandList& cmd)
{
  auto present_images = ctx_->GetBackBufferImages();

  for(auto&& [i, data] : Enumerate(data_))
  {
    data.present_to_read_barrier_ = api::Resource<api::ImageBarrier>(&ctx_->GetDevice())
      .AddImage(present_images[i])
      .AddOldLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddNewLayout(gfx::EImageLayout::PRESENT_SRC);
 
    data.present_to_write_barrier_ = api::Resource<api::ImageBarrier>(&ctx_->GetDevice())
      .AddImage(present_images[i])
      .AddOldLayout(gfx::EImageLayout::PRESENT_SRC)
      .AddNewLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);

    // DepthRenderTarget
    data.depth_image_ = api::Resource<api::Image>(&ctx_->GetDevice())
      .AddExtent(ctx_->GetSurfaceWidth(), ctx_->GetSurfaceHeight(), 1)
      .AddImageUsage(gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT)
      .AddFormatType(gfx::EFormatType::D16_UNORM);
    data.depth_image_view_ = api::Resource<api::ImageView>(&ctx_->GetDevice())
      .AddImage(data.depth_image_->GetHandle())
      .AddFormatType(data.depth_image_->GetFormat());
    api::ImageBarrier* depth_barrier = api::Resource<api::ImageBarrier>(&ctx_->GetDevice())
      .AddImage(*data.depth_image_)
      .AddOldLayout(gfx::EImageLayout::UNDEFINED)
      .AddNewLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    cmd.PushBarrier(*depth_barrier);

    GMDelete(depth_barrier);
  }
}

void gdm::DeferredPass::CreateRenderPass()
{
  pass_ = GMNew api::RenderPass(*device_);

  uint color_idx = 0;
  pass_->AddAttachmentDescription(color_idx)
    .AddFormat(ctx_->GetSurfaceFormat())
    .AddInitLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddFinalLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddRefLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
    .AddStoreOp(gfx::EAttachmentStoreOp::STORE_OP);
  
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

void gdm::DeferredPass::CreateFramebuffer()
{
  uint height = ctx_->GetSurfaceHeight();
  uint width = ctx_->GetSurfaceWidth();

  for (uint i = 0; i < ctx_->GetBackBuffersCount(); ++i)
  {
    api::ImageViews image_views { ctx_->GetBackBufferViews()[i], data_[i].depth_image_view_};
    data_[i].fb_ = GMNew api::Framebuffer(*device_, width, height, *pass_, image_views);
  }
}

void gdm::DeferredPass::CreatePipeline(const api::ImageViews& gbuffer_image_views)
{
  Shader vx_shader("assets/shaders/deferred_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("assets/shaders/deferred_frag.hlsl", gfx::EShaderType::PX);

  auto dsl = api::DescriptorSetLayout(*device_);

  uint pfcb_lights = dsl.AddBinding(0, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::FRAGMENT_STAGE);
  uint sampler     = dsl.AddBinding(1, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_pos   = dsl.AddBinding(2, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_diff  = dsl.AddBinding(3, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  uint gbuff_norm  = dsl.AddBinding(4, 1, gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl.Finalize();

  sampler_ = GMNew api::Sampler(*device_, StdSamplerDesc{});

  for (uint i = 0; i < ctx_->GetBackBuffersCount(); ++i)
  {
    auto* descriptor_set = GMNew api::DescriptorSet(*device_, dsl, ctx_->GetDescriptorPool());
    descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(0, *data_[i].pfcb_uniform_ps_);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(1, *sampler_);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(2, *gbuffer_image_views[0]);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(3, *gbuffer_image_views[1]);
    descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(4, *gbuffer_image_views[2]);
    descriptor_set->Finalize();
    data_[i].descriptor_set_ = descriptor_set;
  }

  float width = static_cast<float>(ctx_->GetSurfaceWidth());
  float height = static_cast<float>(ctx_->GetSurfaceHeight());
  ViewportDesc vp{0, 0, width, height, 0, 1};

  pipeline_ = GMNew api::Pipeline(*device_);
  pipeline_->SetShaderStage(vx_shader, gfx::EShaderType::VX);
  pipeline_->SetShaderStage(px_shader, gfx::EShaderType::PX);
  pipeline_->SetViewportState(vp);
  pipeline_->SetRasterizerState(ReverseRasterizerDesc{});
  pipeline_->SetInputLayout(EmptyInputLayout{});
  pipeline_->SetRenderPass(*pass_);
  pipeline_->SetDescriptorSetLayouts(api::DescriptorSetLayouts{dsl});

  api::BlendState blend_state = api::BlendState(*device_);
  blend_state.AddAttachmentDescription(0)
    .SetEnabled(false)
    .SetColorWriteMask(gfx::R | gfx::G | gfx::B | gfx::A);
  blend_state.Finalize();

  pipeline_->SetBlendState(blend_state);
  pipeline_->Compile();
}

// --public update

void gdm::DeferredPass::Update(uint curr_frame, const CameraEul& camera, const std::vector<ModelLight>& lamps, const std::vector<ModelLight>& flashlights)
{
  data_[curr_frame].pfcb_data_ps_.camera_pos_ = Vec4f(camera.GetPos(), 1.f);
  data_[curr_frame].pfcb_data_ps_.global_ambient_ = Vec4f(0.4f);

  std::vector<ModelLight> lights;

  std::copy(lamps.begin(), lamps.end(), std::back_inserter(lights));
  std::copy(flashlights.begin(), flashlights.end(), std::back_inserter(lights));

  for(auto&& [i, light] : Enumerate(data_[curr_frame].pfcb_data_ps_.lights_))
  {
    if(i >= lights.size())
    {
      light.enabled_ = false;
      continue;
    }
    light.pos_ = Vec4f(lights[i].instance_.tm_.GetCol(3), 1.f);
    light.dir_ = vec3::Normalize(lights[i].instance_.tm_.GetCol(2));
    light.color_ = lights[i].instance_.color_;
    light.enabled_ = lights[i].enabled_;
    light.type_ = static_cast<LightType>(lights[i].instance_.color_.w);
    if (light.type_ == LightType::DIR)
    {
      light.attenuation_const_ = 1.f;
      light.attenuation_linear_ = 0.f;
      light.attenuation_quadr_ = 0.f;
    }
  }
}

void gdm::DeferredPass::Render(api::CommandList& cmd, uint frame_num)
{
  {
    GDM_EVENT_POINT("DeferredUniforms", GDM_LABEL_S(color::Blue));

    cmd.PushBarrier(*data_[frame_num].pfcb_to_write_barrier_);
    data_[frame_num].pfcb_staging_ps_->Map();
    data_[frame_num].pfcb_staging_ps_->CopyDataToGpu(&data_[frame_num].pfcb_data_ps_, 0, 1);
    data_[frame_num].pfcb_staging_ps_->Unmap();
    cmd.CopyBufferToBuffer(*data_[frame_num].pfcb_staging_ps_, *data_[frame_num].pfcb_uniform_ps_, sizeof(DeferredPs_PFCB));
    cmd.PushBarrier(*data_[frame_num].pfcb_to_read_barrier_);
  }
  {
    GDM_EVENT_POINT("DeferredPass", GDM_LABEL_S(color::Blue));

    api::DescriptorSets descriptor_sets {*data_[frame_num].descriptor_set_};
    cmd.PushBarrier(*data_[frame_num].present_to_write_barrier_);    
    cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_, gfx::Offsets{0});      
    cmd.BindPipelineGraphics(*pipeline_);
    cmd.BeginRenderPass(*pass_, *data_[frame_num].fb_, ctx_->GetSurfaceWidth(), ctx_->GetSurfaceHeight());
    cmd.DrawDummy();
    cmd.EndRenderPass();
    cmd.PushBarrier(*data_[frame_num].present_to_read_barrier_);
  }
}