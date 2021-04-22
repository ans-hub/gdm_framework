// *************************************************************
// File:    gbuffer_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gbuffer_pass.h"

#include "render/api.h"
#include "render/defines.h"
#include "render/debug.h"
#include "system/diff_utils.h"
#include "system/event_point.h"
#include "factory/model_factory.h"

// --public

gdm::GbufferPass::GbufferPass(api::Renderer& rdr, api::CommandList& cmd, size_t max_objects, size_t max_materials)
  : rdr_{&rdr}
  , device_{&rdr.GetDevice()}
  , sampler_(*device_, StdSamplerDesc{})
  , width_{rdr_->GetSurfaceWidth()}
  , height_{rdr_->GetSurfaceHeight()}
  , max_objects_{max_objects}
  , max_materials_{max_materials}
  , textures_{ CreateTextures(cmd) }
  , uniforms_{ CreateUniforms(cmd, max_objects) }
  , uniforms_data_ {CreateUniformsData(max_objects) }
  , pipeline_{ CreatePipeline(cmd, textures_.views_, max_materials) }
  // , pipeline_{ CreatePipeline(tag::Simple{}, cmd, textures_.views_, max_materials) }
{ }

void gdm::GbufferPass::Recreate(api::CommandList& cmd)
{
  width_ = rdr_->GetSurfaceWidth();
  height_ = rdr_->GetSurfaceHeight();
  textures_ = CreateTextures(cmd);
  pipeline_ = CreatePipeline(cmd, textures_.views_, max_materials_);
}

//--private

auto gdm::GbufferPass::CreateUniforms(api::CommandList& cmd, uint max_objects) -> Uniforms
{
  Uniforms uniforms {
    .pfcb_uniform_vs_ = {1, false, device_, cmd },
    .pocb_uniform_vs_ = { max_objects, true, device_, cmd },
    .pocb_uniform_ps_ { max_objects, true, device_, cmd }
  };
  uniforms.ub_to_read_barriers_ = {
    uniforms.pfcb_uniform_vs_.GetToReadBarrier(),
    uniforms.pocb_uniform_vs_.GetToReadBarrier(),
    uniforms.pocb_uniform_ps_.GetToReadBarrier()
  };
  uniforms.ub_to_write_barriers_ = {
    uniforms.pfcb_uniform_vs_.GetToWriteBarrier(),
    uniforms.pocb_uniform_vs_.GetToWriteBarrier(),
    uniforms.pocb_uniform_ps_.GetToWriteBarrier()
  };
  cmd.PushBarrier(*uniforms.ub_to_read_barriers_[0]);
  cmd.PushBarrier(*uniforms.ub_to_read_barriers_[1]);
  cmd.PushBarrier(*uniforms.ub_to_read_barriers_[2]);

  return uniforms;
}

auto gdm::GbufferPass::CreateUniformsData(uint max_objects) -> UniformsData
{
  UniformsData uniforms_data;
  uniforms_data.pocb_data_vs_.resize(max_objects);
  uniforms_data.pocb_data_ps_.resize(max_objects);
  return uniforms_data;
}

auto gdm::GbufferPass::CreateTextures(api::CommandList& cmd) -> Textures
{
  const Vec3u whd = {width_, height_, 1u};

  Textures textures {
    .position_ = { gfx::tag::SRRT{}, gfx::EFormatType::F4HALF, whd, cmd, device_ }, 
    .diffuse_ = { gfx::tag::SRRT{}, gfx::EFormatType::F4HALF, whd, cmd, device_ }, 
    .normal_ = { gfx::tag::SRRT{}, gfx::EFormatType::F4HALF, whd, cmd, device_ }, 
    .depth_ = { gfx::tag::DRT{}, gfx::EFormatType::D16_UNORM, whd, cmd, device_ } 
  };

  textures.views_.push_back(&textures.position_.GetImageViewImpl());
  textures.views_.push_back(&textures.diffuse_.GetImageViewImpl());
  textures.views_.push_back(&textures.normal_.GetImageViewImpl());
  textures.views_.push_back(&textures.depth_.GetImageViewImpl());
  textures.to_read_barriers_.push_back(textures.position_.GetToReadBarrier());
  textures.to_read_barriers_.push_back(textures.diffuse_.GetToReadBarrier());
  textures.to_read_barriers_.push_back(textures.normal_.GetToReadBarrier());
  textures.to_write_barriers_.push_back(textures.position_.GetToWriteBarrier());
  textures.to_write_barriers_.push_back(textures.diffuse_.GetToWriteBarrier());
  textures.to_write_barriers_.push_back(textures.normal_.GetToWriteBarrier());

  return textures;
}

auto gdm::GbufferPass::CreatePipeline(api::CommandList& cmd, const api::ImageViews& views, size_t materials_cnt) -> Pipeline
{
  Pipeline pipeline;
  pipeline.api_pass_ = CreateRenderPass();
  pipeline.api_fb_ = CreateFramebuffer(pipeline.api_pass_.get(), views);
  pipeline.api_descriptor_set_layout_ = CreateDescriptorSetLayout(materials_cnt);
  pipeline.api_descriptor_set_ = CreateDescriptorSet(materials_cnt, pipeline.api_descriptor_set_layout_.get(), uniforms_);
  pipeline.api_pipeline_ = CompilePipeline(pipeline.api_pass_.get(), pipeline.api_descriptor_set_layout_.get());

  return pipeline;
}

auto gdm::GbufferPass::CreateRenderPass() -> std::unique_ptr<api::RenderPass>
{
  auto render_pass = std::make_unique<api::RenderPass>(*device_);

  for (int i = 0; i < 3; ++i)
  {
    render_pass->AddAttachmentDescription(i)
      .AddFormat(gfx::EFormatType::F4HALF)
      .AddInitLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddFinalLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddRefLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);
  }
  
  uint gbuff_depth_idx = 3;
  render_pass->AddAttachmentDescription(gbuff_depth_idx)
    .AddFormat(gfx::EFormatType::D16_UNORM)
    .AddInitLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .AddFinalLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    .AddRefLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  uint subpass_idx = render_pass->CreateSubpass(gfx::EQueueType::GRAPHICS);
  render_pass->AddSubpassColorAttachments(subpass_idx, api::Attachments{0,1,2});
  render_pass->AddSubpassDepthAttachments(subpass_idx, api::Attachment{3});
  render_pass->Finalize();

  return render_pass;
}

auto gdm::GbufferPass::CreateFramebuffer(api::RenderPass* pass, const api::ImageViews& views) -> std::unique_ptr<api::Framebuffer>
{
  return std::make_unique<api::Framebuffer>(*device_, width_, height_, *pass, views);
}

auto gdm::GbufferPass::CreateDescriptorSetLayout(size_t materials_cnt) -> std::unique_ptr<api::DescriptorSetLayout>
{
  auto dsl = std::make_unique<api::DescriptorSetLayout>(*device_);

  dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(1, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(2, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(3, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(4, static_cast<uint>(materials_cnt), gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::VARIABLE_DESCRIPTOR | gfx::PARTIALLY_BOUND);
  dsl->Finalize();
  
  return dsl;
}

auto gdm::GbufferPass::CreateDescriptorSet(size_t materials_cnt, api::DescriptorSetLayout* dsl, Uniforms& uniforms) -> std::unique_ptr<api::DescriptorSet>
{
  auto ds = std::make_unique<api::DescriptorSet>(*device_, *dsl, rdr_->GetDescriptorPool());

  ds->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, uniforms.pfcb_uniform_vs_.GetImpl());
  ds->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, uniforms.pocb_uniform_vs_.GetImpl());
  ds->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(2, uniforms.pocb_uniform_ps_.GetImpl());
  ds->UpdateContent<gfx::EResourceType::SAMPLER>(3, sampler_);
  ds->Finalize();

  return ds;
}

auto gdm::GbufferPass::CompilePipeline(api::RenderPass* pass, api::DescriptorSetLayout* dsl) -> std::unique_ptr<api::Pipeline>
{
  Shader vx_shader("assets/shaders/gbuffer_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("assets/shaders/gbuffer_frag.hlsl", gfx::EShaderType::PX);
  
  float width = (float)width_;
  float height = (float)height_;

  ViewportDesc vp{0, height, width, -height, 0, 1};

  auto pipeline = std::make_unique<api::Pipeline>(*device_);
  pipeline->SetShaderStage(vx_shader, gfx::EShaderType::VX);
  pipeline->SetShaderStage(px_shader, gfx::EShaderType::PX);
  pipeline->SetViewportState(vp);
  pipeline->SetRasterizerState(StdRasterizerDesc{});
  pipeline->SetInputLayout(StdInputLayout{});
  pipeline->SetRenderPass(*pass);
  pipeline->SetDescriptorSetLayouts(api::DescriptorSetLayouts{*dsl});

  const uint depth_attachments_cnt = 1;
  
  api::BlendState blend_state = api::BlendState(*device_);
  
  for (uint i = 0; i < pass->GetPassAttachmentsCount() - depth_attachments_cnt; ++i)
  {
    blend_state.AddAttachmentDescription(i)
      .SetEnabled(false)
      .SetColorWriteMask(gfx::R | gfx::G | gfx::B | gfx::A);
  }

  blend_state.Finalize();

  pipeline->SetBlendState(blend_state);
  pipeline->Compile();

  return pipeline;
}

// --public Updates

void gdm::GbufferPass::UpdateUniforms(api::CommandList& cmd, uint max_objects)
{
  GDM_EVENT_POINT("GbufferUniform", GDM_LABEL_S(color::Black));

  uniforms_.pfcb_uniform_vs_.Update(cmd, &uniforms_data_.pfcb_data_vs_, 0, 1);
  uniforms_.pocb_uniform_vs_.Update(cmd, uniforms_data_.pocb_data_vs_.data(), 0, uniforms_data_.pocb_data_vs_.size());
  uniforms_.pocb_uniform_ps_.Update(cmd, uniforms_data_.pocb_data_ps_.data(), 0, uniforms_data_.pocb_data_ps_.size());
}

void gdm::GbufferPass::UpdateUniformsData(const CameraEul& camera, const std::vector<ModelInstance*>& renderable_models)
{
  Mat4f view = camera.GetViewMx();
  Mat4f proj = camera.GetProjectionMx();
  uniforms_data_.pfcb_data_vs_.u_view_proj_ = proj * view;
  uniforms_data_.pfcb_data_vs_.u_cam_pos_ = camera.GetPos();

  uint mesh_number = 0;
  for (const auto& model_instance : renderable_models)
  {
    AbstractModel* model = ModelFactory::Get(model_instance->handle_);
    for (auto&& [i, mesh_handle] : Enumerate(model->meshes_))
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
      uniforms_data_.pocb_data_vs_[mesh_number].u_model_ = model_instance->tm_;
      uniforms_data_.pocb_data_vs_[mesh_number].u_color_ = model_instance->color_;
      uniforms_data_.pocb_data_ps_[mesh_number].u_material_index_ = material->index_;
      uniforms_data_.pocb_data_ps_[mesh_number].u_material_props_ = material->props_;
      ++mesh_number;
    }
  }
}

void gdm::GbufferPass::UpdateDescriptorSet(const api::ImageViews& renderable_materials)
{
  pipeline_.api_descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, uniforms_.pfcb_uniform_vs_.GetImpl());
  pipeline_.api_descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, uniforms_.pocb_uniform_vs_.GetImpl());
  pipeline_.api_descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(2, uniforms_.pocb_uniform_ps_.GetImpl());
  pipeline_.api_descriptor_set_->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(4, renderable_materials);
  pipeline_.api_descriptor_set_->Finalize();
}

void gdm::GbufferPass::Render(api::CommandList& cmd, const std::vector<ModelInstance*>& renderable_models)
{
  GDM_EVENT_POINT("GbufferPass", GDM_LABEL_S(color::Black));

  cmd.PushBarriers(textures_.to_write_barriers_);
  cmd.BindPipelineGraphics(*pipeline_.api_pipeline_);
  cmd.BeginRenderPass(*pipeline_.api_pass_, *pipeline_.api_fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());

  int mesh_number = 0;
  for (const auto& model_instance : renderable_models)
  {
    AbstractModel* model = ModelFactory::Get(model_instance->handle_);
    for (auto mesh_handle : model->meshes_)
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      uint offset = sizeof(GbufferVs_POCB) * mesh_number++;
      api::DescriptorSets descriptor_sets {*pipeline_.api_descriptor_set_};
      
      cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_.api_pipeline_, gfx::Offsets{offset, offset});      
      cmd.BindVertexBuffer(*mesh->GetVertexBuffer<api::Buffer>());
      cmd.BindIndexBuffer(*mesh->GetIndexBuffer<api::Buffer>());
      cmd.DrawIndexed(mesh->faces_);
    }
  }
  
  cmd.EndRenderPass();
  cmd.PushBarriers(textures_.to_read_barriers_);
}
