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

struct GbufferPass
{
  GbufferPass(api::Renderer& ctx, api::CommandList& cmd, size_t max_objects, size_t max_materials);

public:
  void UpdateUniforms(api::CommandList& cmd, uint max_objects);
  void UpdateUniformsData(const CameraEul& camera, const std::vector<ModelInstance*>& renderable_models);
  void UpdateDescriptorSet(const api::ImageViews& renderable_materials);
  void Render(api::CommandList& cmd, const std::vector<ModelInstance*>& renderable_models);
  void Recreate(api::CommandList& cmd);
  
  auto GetTextureViews() const -> const api::ImageViews& { return textures_.views_; }

private:
  struct Pipeline
  {
    std::unique_ptr<api::RenderPass> api_pass_;
    std::unique_ptr<api::Framebuffer> api_fb_;
    std::unique_ptr<api::DescriptorSetLayout> api_descriptor_set_layout_;
    std::unique_ptr<api::DescriptorSet> api_descriptor_set_;  
    std::unique_ptr<api::Pipeline> api_pipeline_;
  };
  struct Textures
  {
    gfx::Texture position_;
    gfx::Texture diffuse_;
    gfx::Texture normal_;
    gfx::Texture depth_;
    std::vector<api::ImageView*> views_;
    std::vector<api::ImageBarrier*> to_read_barriers_;
    std::vector<api::ImageBarrier*> to_write_barriers_;
  };
  struct Uniforms
  {
    gfx::UniformBuffer<GbufferVs_PFCB> pfcb_uniform_vs_;
    gfx::UniformBuffer<GbufferVs_POCB> pocb_uniform_vs_;
    gfx::UniformBuffer<GbufferPs_POCB> pocb_uniform_ps_;
    std::vector<api::BufferBarrier*> ub_to_read_barriers_;
    std::vector<api::BufferBarrier*> ub_to_write_barriers_;
  };
  struct UniformsData
  {
    std::vector<GbufferVs_POCB> pocb_data_vs_;
    std::vector<GbufferPs_POCB> pocb_data_ps_;
    GbufferVs_PFCB pfcb_data_vs_;
  };

private:
  auto CreateUniformsData(uint max_objects) -> UniformsData;
  auto CreateUniforms(api::CommandList& cmd, uint max_objects) -> Uniforms;
  auto CreateTextures(api::CommandList& cmd) -> Textures;
  auto CreatePipeline(api::CommandList& cmd, const api::ImageViews& views, size_t materials_cnt) -> Pipeline;
  auto CreateRenderPass() -> std::unique_ptr<api::RenderPass>;
  auto CreateFramebuffer(api::RenderPass* pass, const api::ImageViews& views) -> std::unique_ptr<api::Framebuffer>;
  auto CreateDescriptorSetLayout(size_t materials_cnt) -> std::unique_ptr<api::DescriptorSetLayout>;
  auto CreateDescriptorSet(size_t materials_cnt, api::DescriptorSetLayout* dsl, Uniforms& uniforms) -> std::unique_ptr<api::DescriptorSet>;
  auto CompilePipeline(api::RenderPass* pass, api::DescriptorSetLayout* dsl) -> std::unique_ptr<api::Pipeline>;

private:
  api::Renderer* ctx_;
  api::Device* device_;
  api::Sampler sampler_;
  uint width_;
  uint height_;
  size_t max_objects_;
  size_t max_materials_;
  Textures textures_;
  Uniforms uniforms_;
  UniformsData uniforms_data_;
  Pipeline pipeline_;

}; // struct GbufferPass

} // namespace gdm

#endif // GFX_GBUFFER_DESC_H