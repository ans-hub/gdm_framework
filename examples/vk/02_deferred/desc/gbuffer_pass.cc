// *************************************************************
// File:    gbuffer_pass.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gbuffer_pass.h"

#include "render/api.h"
#include "render/defines.h"
#include "system/diff_utils.h"
#include "data/model_factory.h"

// --public create

void gdm::GbufferPass::CreateUniforms(api::CommandList& cmd, uint max_objects)
{
  data_.pocb_data_vs_.resize(max_objects);
  data_.pocb_data_ps_.resize(max_objects);
  data_.pfcb_staging_vs_ = GMNew api::Buffer(device_, sizeof(GbufferVs_PFCB) * 1, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE);
  data_.pocb_staging_vs_ = GMNew api::Buffer(device_, sizeof(GbufferVs_POCB) * max_objects, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE);
  data_.pocb_staging_ps_ = GMNew api::Buffer(device_, sizeof(GbufferPs_POCB) * max_objects, gfx::TRANSFER_SRC, gfx::HOST_VISIBLE);
  data_.pfcb_uniform_vs_ = GMNew api::Buffer(device_, sizeof(GbufferVs_PFCB) * 1, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  data_.pocb_uniform_vs_ = GMNew api::Buffer(device_, sizeof(GbufferVs_POCB) * max_objects, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  data_.pocb_uniform_ps_ = GMNew api::Buffer(device_, sizeof(GbufferPs_POCB) * max_objects, gfx::TRANSFER_DST | gfx::UNIFORM, gfx::DEVICE_LOCAL);
  data_.to_read_barriers_.resize(3);
  data_.to_write_barriers_.resize(3);
  data_.to_read_barriers_[0] = GMNew api::BufferBarrier(device_, *data_.pfcb_uniform_vs_, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  data_.to_read_barriers_[1] = GMNew api::BufferBarrier(device_, *data_.pocb_uniform_vs_, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  data_.to_read_barriers_[2] = GMNew api::BufferBarrier(device_, *data_.pocb_uniform_ps_, gfx::EAccess::TRANSFER_WRITE, gfx::EAccess::UNIFORM_READ);
  data_.to_write_barriers_[0] = GMNew api::BufferBarrier(device_, *data_.pfcb_uniform_vs_, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  data_.to_write_barriers_[1] = GMNew api::BufferBarrier(device_, *data_.pocb_uniform_vs_, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  data_.to_write_barriers_[2] = GMNew api::BufferBarrier(device_, *data_.pocb_uniform_ps_, gfx::EAccess::UNIFORM_READ, gfx::EAccess::TRANSFER_WRITE);
  
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

void gdm::GbufferPass::CreateDescriptorSet(const api::ImageViews& materials)
{
  auto* dsl = GMNew api::DescriptorSetLayout(*device_);

  dsl->AddBinding(0, 1, gfx::EResourceType::UNIFORM_BUFFER, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(1, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::VERTEX_STAGE);
  dsl->AddBinding(2, 1, gfx::EResourceType::UNIFORM_DYNAMIC, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(3, 1, gfx::EResourceType::SAMPLER, gfx::EShaderStage::FRAGMENT_STAGE);
  dsl->AddBinding(4, static_cast<uint>(materials.size()), gfx::EResourceType::SAMPLED_IMAGE, gfx::EShaderStage::FRAGMENT_STAGE, gfx::EBindingFlags::VARIABLE_DESCRIPTOR);
  dsl->Finalize();

  data_.sampler_ = GMNew api::Sampler(*device_, StdSamplerState{});

  auto* descriptor_set = GMNew api::DescriptorSet(*device_, *dsl, rdr_->GetDescriptorPool());
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_BUFFER>(0, *data_.pfcb_uniform_vs_);
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(1, *data_.pocb_uniform_vs_);
  descriptor_set->UpdateContent<gfx::EResourceType::UNIFORM_DYNAMIC>(2, *data_.pocb_uniform_ps_);
  descriptor_set->UpdateContent<gfx::EResourceType::SAMPLER>(3, *data_.sampler_);
  descriptor_set->UpdateContent<gfx::EResourceType::SAMPLED_IMAGE>(4, materials);
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

// --public Updates

void gdm::GbufferPass::UpdateUniforms(api::CommandList& cmd, uint max_objects)
{
  cmd.PushBarrier(*data_.to_write_barriers_[0]);
  cmd.PushBarrier(*data_.to_write_barriers_[1]);
  cmd.PushBarrier(*data_.to_write_barriers_[2]);

  data_.pfcb_staging_vs_->Map();
  data_.pfcb_staging_vs_->CopyDataToGpu(&data_.pfcb_data_vs_, 1);
  data_.pfcb_staging_vs_->Unmap();
  cmd.CopyBufferToBuffer(*data_.pfcb_staging_vs_, *data_.pfcb_uniform_vs_, sizeof(GbufferVs_PFCB));
  
  data_.pocb_staging_vs_->CopyDataToGpu(data_.pocb_data_vs_.data(), data_.pocb_data_vs_.size());
  uint pocb_size_vs = static_cast<uint>(sizeof(GbufferVs_POCB) * data_.pocb_data_vs_.size());
  cmd.CopyBufferToBuffer(*data_.pocb_staging_vs_, *data_.pocb_uniform_vs_, pocb_size_vs);

  data_.pocb_staging_ps_->CopyDataToGpu(data_.pocb_data_ps_.data(), data_.pocb_data_ps_.size());
  uint pocb_size_ps = static_cast<uint>(sizeof(GbufferPs_POCB) * data_.pocb_data_ps_.size());
  cmd.CopyBufferToBuffer(*data_.pocb_staging_ps_, *data_.pocb_uniform_ps_, pocb_size_ps);

  cmd.PushBarrier(*data_.to_read_barriers_[0]);
  cmd.PushBarrier(*data_.to_read_barriers_[1]);
  cmd.PushBarrier(*data_.to_read_barriers_[2]);
}

void gdm::GbufferPass::UpdateUniformsData(const CameraEul& camera, const std::vector<ModelInstance>& renderable_models)
{
  Mat4f view = camera.GetViewMx();
  Mat4f proj = camera.GetProjectionMx();
  data_.pfcb_data_vs_.u_view_proj_ = proj * view;
  data_.pfcb_data_vs_.u_cam_pos_ = camera.GetPos();

  uint mesh_number = 0;
  for (const auto& model_instance : renderable_models)
  {
    AbstractModel* model = ModelFactory::Get(model_instance.handle_);
    for (auto&& [i, mesh_handle] : Enumerate(model->meshes_))
    {
      AbstractMesh* mesh = MeshFactory::Get(mesh_handle);
      AbstractMaterial* material = MaterialFactory::Get(mesh->material_);
      data_.pocb_data_vs_[mesh_number].u_model_ = model_instance.tm_;
      data_.pocb_data_vs_[mesh_number].u_color_ = model_instance.color_;
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

void gdm::GbufferPass::Draw(api::CommandList& cmd, const std::vector<ModelInstance>& renderable_models)
{
  for(auto* barrier : data_.image_barriers_to_write_)
    cmd.PushBarrier(*barrier);

  cmd.BindPipelineGraphics(*pipeline_);
  cmd.BeginRenderPass(*pass_, *data_.fb_, rdr_->GetSurfaceWidth(), rdr_->GetSurfaceHeight());

  int mesh_number = 0;
  for (const auto& model_instance : renderable_models)
  {
    AbstractModel* model = ModelFactory::Get(model_instance.handle_);
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
