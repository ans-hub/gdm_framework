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

gdm::GbufferPass::GbufferPass(api::Renderer& rdr, int width, int height)
  : rdr_{&rdr}
  , device_{&rdr.GetDevice()}
  , data_()
  , w_{width}
  , h_{height}
{ }

gdm::GbufferPass::~GbufferPass()
{
  Cleanup();
}

void gdm::GbufferPass::Cleanup()
{
  GMDelete(data_.pfcb_staging_vs_);
  GMDelete(data_.pocb_staging_ps_);
  GMDelete(data_.pfcb_uniform_vs_);
  GMDelete(data_.pocb_uniform_vs_);
  GMDelete(data_.pocb_uniform_ps_);

  for (auto* barrier : data_.to_read_barriers_)
    GMDelete(barrier);
  for (auto* barrier : data_.to_write_barriers_)
    GMDelete(barrier);

  GMDelete(data_.sampler_);

  for (auto* image : data_.images_)
    GMDelete(image);
  for (auto* image_view : data_.image_views_)
    GMDelete(image_view);
  for (auto* barrier : data_.image_barriers_to_read_)
    GMDelete(barrier);
  for (auto* barrier : data_.image_barriers_to_write_)
    GMDelete(barrier);

  GMDelete(data_.fb_);
  GMDelete(data_.descriptor_set_layout_);
  GMDelete(data_.descriptor_set_);

  GMDelete(sampler_);
  GMDelete(pass_);
  GMDelete(pipeline_);
}

void gdm::GbufferPass::CreateUniforms(api::CommandList& cmd, uint max_objects)
{
  data_.pocb_data_vs_.resize(max_objects);
  data_.pocb_data_ps_.resize(max_objects);

  data_.pfcb_staging_vs_ = gfx::Resource<api::Buffer>(device_, sizeof(GbufferVs_PFCB) * 1)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE);
  data_.pocb_staging_vs_ = gfx::Resource<api::Buffer>(device_, sizeof(GbufferVs_POCB) * max_objects)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE);
  data_.pocb_staging_ps_ = gfx::Resource<api::Buffer>(device_, sizeof(GbufferPs_POCB) * max_objects)
    .AddUsage(gfx::TRANSFER_SRC)
    .AddMemoryType(gfx::HOST_VISIBLE);
  data_.pfcb_uniform_vs_ = gfx::Resource<api::Buffer>(device_, sizeof(GbufferVs_PFCB) * 1)
    .AddUsage(gfx::TRANSFER_DST | gfx::UNIFORM)
    .AddMemoryType(gfx::DEVICE_LOCAL);
  data_.pocb_uniform_vs_ = gfx::Resource<api::Buffer>(device_, sizeof(GbufferVs_POCB) * max_objects)
    .AddUsage(gfx::TRANSFER_DST | gfx::UNIFORM)
    .AddMemoryType(gfx::DEVICE_LOCAL);
  data_.pocb_uniform_ps_ = gfx::Resource<api::Buffer>(device_, sizeof(GbufferPs_POCB) * max_objects)
    .AddUsage(gfx::TRANSFER_DST | gfx::UNIFORM)
    .AddMemoryType(gfx::DEVICE_LOCAL);

  data_.to_read_barriers_.resize(3);
  data_.to_write_barriers_.resize(3);

  data_.to_read_barriers_[0] = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_.pfcb_uniform_vs_)
    .AddOldAccess(gfx::EAccess::TRANSFER_WRITE)
    .AddNewAccess(gfx::EAccess::UNIFORM_READ);
  data_.to_read_barriers_[1] = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_.pfcb_uniform_vs_)
    .AddOldAccess(gfx::EAccess::TRANSFER_WRITE)
    .AddNewAccess(gfx::EAccess::UNIFORM_READ);
  data_.to_read_barriers_[2] = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_.pfcb_uniform_vs_)
    .AddOldAccess(gfx::EAccess::TRANSFER_WRITE)
    .AddNewAccess(gfx::EAccess::UNIFORM_READ);
  data_.to_write_barriers_[0] = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_.pfcb_uniform_vs_)
    .AddOldAccess(gfx::EAccess::UNIFORM_READ)
    .AddNewAccess(gfx::EAccess::TRANSFER_WRITE);
  data_.to_write_barriers_[1] = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_.pfcb_uniform_vs_)
    .AddOldAccess(gfx::EAccess::UNIFORM_READ)
    .AddNewAccess(gfx::EAccess::TRANSFER_WRITE);
  data_.to_write_barriers_[2] = gfx::Resource<api::BufferBarrier>(device_)
    .AddBuffer(*data_.pfcb_uniform_vs_)
    .AddOldAccess(gfx::EAccess::UNIFORM_READ)
    .AddNewAccess(gfx::EAccess::TRANSFER_WRITE);
  
  cmd.PushBarrier(*data_.to_read_barriers_[0]);
  cmd.PushBarrier(*data_.to_read_barriers_[1]);
  cmd.PushBarrier(*data_.to_read_barriers_[2]);

  bool map_and_never_unmap_0 = [&](){ data_.pocb_staging_vs_->Map(); return true; }();
  bool map_and_never_unmap_1 = [&](){ data_.pocb_staging_ps_->Map(); return true; }();
}

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
    data_.images_[i] = gfx::Resource<api::Image2D>(device_, w_, h_)
      .AddFormatType(gfx::EFormatType::F4HALF)
      .AddImageUsage(gfx::EImageUsage::COLOR_ATTACHMENT | gfx::EImageUsage::SAMPLED);
    data_.image_views_[i] = gfx::Resource<api::ImageView>(*device_)
      .AddImage(*data_.images_[i])
      .AddFormatType(data_.images_[i]->GetFormat());
    data_.image_barriers_to_read_[i] = gfx::Resource<api::ImageBarrier>()
      .AddImage(*data_.images_[i])
      .AddOldLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL)
      .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);
    data_.image_barriers_to_write_[i] = gfx::Resource<api::ImageBarrier>()
      .AddImage(*data_.images_[i])
      .AddOldLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL)
      .AddNewLayout(gfx::EImageLayout::COLOR_ATTACHMENT_OPTIMAL);

    api::ImageBarrier* init_barrier = gfx::Resource<api::ImageBarrier>()
      .AddImage(*data_.images_[i])
      .AddOldLayout(gfx::EImageLayout::UNDEFINED)
      .AddNewLayout(gfx::EImageLayout::SHADER_READ_OPTIMAL);

    cmd.PushBarrier(*init_barrier);

    GMDelete(init_barrier);
  }

  data_.images_[3] = gfx::Resource<api::Image2D>(device_, w_, h_)
    .AddFormatType(gfx::EFormatType::D16_UNORM)
    .AddImageUsage(gfx::EImageUsage::DEPTH_STENCIL_ATTACHMENT);

  data_.image_views_[3] = gfx::Resource<api::ImageView>(*device_)
    .AddImage(*data_.images_.back())
    .AddFormatType(data_.images_.back()->GetFormat());

  api::ImageBarrier* depth_barrier = gfx::Resource<api::ImageBarrier>()
    .AddImage(*data_.images_[3])
    .AddOldLayout(gfx::EImageLayout::UNDEFINED)
    .AddNewLayout(gfx::EImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

  cmd.PushBarrier(*depth_barrier);

  GMDelete(depth_barrier);
}

void gdm::GbufferPass::CreateFramebuffer()
{
  data_.fb_ = GMNew api::Framebuffer(*device_, w_, h_, *pass_, data_.image_views_);
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

void gdm::GbufferPass::CreateDescriptorSet(size_t materials_cnt)
{
  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(1, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(2, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(3, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(4, static_cast<uint>(materials_cnt), gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::VARIABLE_DESCRIPTOR | gfx::PARTIALLY_BOUND);
  dsl->Finalize();

  data_.sampler_ = GMNew api::Sampler(*device_, StdSamplerDesc{});

  auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_.pfcb_uniform_vs_);
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, *data_.pocb_uniform_vs_);
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(2, *data_.pocb_uniform_ps_);
  descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(3, *data_.sampler_);
  descriptor_set->Finalize();
  
  data_.descriptor_set_layout_ = dsl;
  data_.descriptor_set_ = descriptor_set;
}

void gdm::GbufferPass::CreatePipeline()
{
  Shader vx_shader("assets/shaders/gbuffer_vert.hlsl", gfx::EShaderType::VX);
  Shader px_shader("assets/shaders/gbuffer_frag.hlsl", gfx::EShaderType::PX);
  
  float width = (float)w_;
  float height = (float)h_;

  ViewportDesc vp{0, height, width, -height, 0, 1};

  pipeline_ = GMNew api::Pipeline(*device_);
  pipeline_->SetShaderStage(vx_shader, gfx::EShaderType::VX);
  pipeline_->SetShaderStage(px_shader, gfx::EShaderType::PX);
  pipeline_->SetViewportState(vp);
  pipeline_->SetRasterizerState(StdRasterizerDesc{});
  pipeline_->SetInputLayout(StdInputLayout{});
  pipeline_->SetRenderPass(*pass_);
  pipeline_->SetDescriptorSetLayouts(api::DescriptorSetLayouts{*data_.descriptor_set_layout_});

  const uint depth_attachments_cnt = 1;
  
  api::BlendState blend_state = api::BlendState(*device_);
  
  for (uint i = 0; i < pass_->GetPassAttachmentsCount() - depth_attachments_cnt; ++i)
  {
    blend_state.AddAttachmentDescription(i)
      .SetEnabled(false)
      .SetColorWriteMask(gfx::R | gfx::G | gfx::B | gfx::A);
  }

  blend_state.Finalize();

  pipeline_->SetBlendState(blend_state);
  pipeline_->Compile();
}

// --public Updates

void gdm::GbufferPass::UpdateUniforms(api::CommandList& cmd, uint max_objects)
{
  GDM_EVENT_POINT("GbufferUniform", GDM_LABEL_S(color::Black));

  cmd.PushBarrier(*data_.to_write_barriers_[0]);
  cmd.PushBarrier(*data_.to_write_barriers_[1]);
  cmd.PushBarrier(*data_.to_write_barriers_[2]);

  data_.pfcb_staging_vs_->Map();
  data_.pfcb_staging_vs_->CopyDataToGpu(&data_.pfcb_data_vs_, 0, 1);
  data_.pfcb_staging_vs_->Unmap();
  cmd.CopyBufferToBuffer(*data_.pfcb_staging_vs_, *data_.pfcb_uniform_vs_, sizeof(GbufferVs_PFCB));
  
  data_.pocb_staging_vs_->CopyDataToGpu(data_.pocb_data_vs_.data(), 0, data_.pocb_data_vs_.size());
  uint pocb_size_vs = static_cast<uint>(sizeof(GbufferVs_POCB) * data_.pocb_data_vs_.size());
  cmd.CopyBufferToBuffer(*data_.pocb_staging_vs_, *data_.pocb_uniform_vs_, pocb_size_vs);

  data_.pocb_staging_ps_->CopyDataToGpu(data_.pocb_data_ps_.data(), 0, data_.pocb_data_ps_.size());
  uint pocb_size_ps = static_cast<uint>(sizeof(GbufferPs_POCB) * data_.pocb_data_ps_.size());
  cmd.CopyBufferToBuffer(*data_.pocb_staging_ps_, *data_.pocb_uniform_ps_, pocb_size_ps);

  cmd.PushBarrier(*data_.to_read_barriers_[0]);
  cmd.PushBarrier(*data_.to_read_barriers_[1]);
  cmd.PushBarrier(*data_.to_read_barriers_[2]);
}

void gdm::GbufferPass::UpdateUniformsData(const CameraEul& camera, const std::vector<ModelInstance*>& renderable_models)
{
  Mat4f view = camera.GetViewMx();
  Mat4f proj = camera.GetProjectionMx();
  data_.pfcb_data_vs_.u_view_proj_ = proj * view;
  data_.pfcb_data_vs_.u_cam_pos_ = camera.GetPos();

  uint mesh_number = 0;
  for (const auto& model_instance : renderable_models)
  {
    AbstractModel* model = ModelFactory::Get(model_instance->handle_);
    for (auto&& [i, mesh_handle] : Enumerate(model->meshes_))
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
      data_.pocb_data_vs_[mesh_number].u_model_ = model_instance->tm_;
      data_.pocb_data_vs_[mesh_number].u_color_ = model_instance->color_;
      data_.pocb_data_ps_[mesh_number].u_material_index_ = material->index_;
      data_.pocb_data_ps_[mesh_number].u_material_props_ = material->props_;
      ++mesh_number;
    }
  }
}

void gdm::GbufferPass::UpdateDescriptorSet(const api::ImageViews& renderable_materials)
{
  data_.descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_.pfcb_uniform_vs_);
  data_.descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, *data_.pocb_uniform_vs_);
  data_.descriptor_set_->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(2, *data_.pocb_uniform_ps_);
  data_.descriptor_set_->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(4, renderable_materials);
  data_.descriptor_set_->Finalize();
}

void gdm::GbufferPass::Draw(api::CommandList& cmd, const std::vector<ModelInstance*>& renderable_models)
{
  GDM_EVENT_POINT("GbufferPass", GDM_LABEL_S(color::Black));

  for(auto* barrier : data_.image_barriers_to_write_)
    cmd.PushBarrier(*barrier);

  cmd.BindPipelineGraphics(*pipeline_);
  cmd.BeginRenderPass(*pass_, *data_.fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());

  int mesh_number = 0;
  for (const auto& model_instance : renderable_models)
  {
    AbstractModel* model = ModelFactory::Get(model_instance->handle_);
    for (auto mesh_handle : model->meshes_)
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      uint offset = sizeof(GbufferVs_POCB) * mesh_number++;
      api::DescriptorSets descriptor_sets {*data_.descriptor_set_};
      
      cmd.BindDescriptorSetGraphics(descriptor_sets, *pipeline_, gfx::Offsets{offset, offset});      
      cmd.BindVertexBuffer(*mesh->GetVertexBuffer<api::Buffer>());
      cmd.BindIndexBuffer(*mesh->GetIndexBuffer<api::Buffer>());
      cmd.DrawIndexed(mesh->faces_);
    }
  }
  
  cmd.EndRenderPass();
  for(auto* barrier : data_.image_barriers_to_read_)
    cmd.PushBarrier(*barrier);
}
