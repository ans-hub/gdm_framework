// *************************************************************
// File:    gbuffer_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_GBUFFER_DESC_H
#define GFX_GBUFFER_DESC_H

#include <set>

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "render/texture.h"
#include "render/uniform_buffer.h"

#include "engine/camera_eul.h"

#include "math/matrix.h"
#include "math/vector4.h"

#include "factory/model_factory.h"
#include "factory/material_factory.h"

#include "render/desc/sampler_desc.h"
#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"

namespace gdm {

struct alignas(64) GbufferVs_PFCB
{
  alignas(16) Mat4f u_view_proj_;
  alignas(16) Vec3f u_cam_pos_;
  float dummy;

  static const gfx::UboType v_ubo_type_ = gfx::EUboType::PER_FRAME; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::VX; 

}; // struct GbufferVs_PFCB

struct alignas(64) GbufferVs_POCB
{
  alignas(16) Mat4f u_model_;
  alignas(16) Vec4f u_color_;

  static const gfx::UboType v_type_ = gfx::EUboType::PER_OBJECT; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::VX; 
  
}; // struct GbufferVs_POCB

struct alignas(64) GbufferPs_POCB
{
  alignas(16) AbstractMaterial::Props u_material_props_;
  alignas(16) unsigned int u_material_index_;

  static const gfx::UboType v_type_ = gfx::EUboType::PER_OBJECT; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::PX; 

}; // struct GbufferPs_POCB

struct GbufferPassData
{
  gfx::UniformBuffer<GbufferVs_PFCB>* pfcb_uniform_vs_;
  gfx::UniformBuffer<GbufferVs_POCB>* pocb_uniform_vs_;
  gfx::UniformBuffer<GbufferPs_POCB>* pocb_uniform_ps_;
  std::vector<api::BufferBarrier*> ub_to_read_barriers_;
  std::vector<api::BufferBarrier*> ub_to_write_barriers_;
  gfx::Texture* tex_position_;
  gfx::Texture* tex_diffuse_;
  gfx::Texture* tex_normal_;
  gfx::Texture* tex_depth_;
  std::vector<api::ImageView*> tex_views_;
  std::vector<api::ImageBarrier*> tex_to_read_barriers_;
  std::vector<api::ImageBarrier*> tex_to_write_barriers_;
  api::Sampler* sampler_;
  api::Framebuffer* fb_;
  api::DescriptorSetLayout* descriptor_set_layout_;
  api::DescriptorSet* descriptor_set_;  

  GbufferVs_PFCB pfcb_data_vs_;
  std::vector<GbufferVs_POCB> pocb_data_vs_;
  std::vector<GbufferPs_POCB> pocb_data_ps_;
};

struct GbufferPass
{
  GbufferPass(api::Renderer& rdr, int width, int height);
  ~GbufferPass();
  
  void Cleanup();

  api::Renderer* rdr_ = nullptr;
  api::Device* device_ = nullptr;
  api::Sampler* sampler_ = nullptr;
  api::RenderPass* pass_ = nullptr;
  api::Pipeline* pipeline_ = nullptr;

  GbufferPassData data_;

  int w_;
  int h_;

  void CreateUniforms(api::CommandList& cmd, uint max_objects);
  void CreateImages(api::CommandList& cmd);
  void CreateFramebuffer();
  void CreateRenderPass();
  void CreateDescriptorSet(size_t materials_cnt);
  void CreatePipeline();
  
  void UpdateUniforms(api::CommandList& cmd, uint max_objects);
  void UpdateUniformsData(const CameraEul& camera, const std::vector<ModelInstance*>& renderable_models);
  void UpdateDescriptorSet(const api::ImageViews& renderable_materials);
  void Draw(api::CommandList& cmd, const std::vector<ModelInstance*>& renderable_models);
};

} // namespace gdm

#endif // GFX_GBUFFER_DESC_H