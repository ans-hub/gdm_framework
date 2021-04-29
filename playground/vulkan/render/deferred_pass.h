// *************************************************************
// File:    deferred_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_DEFERRED_DESC_H
#define GFX_DEFERRED_DESC_H

#include <set>

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/colors.h"
#include "render/shader.h"
#include "engine/camera_eul.h"

#include "factory/material_factory.h"
#include "factory/model_factory.h"

#include "render/desc/sampler_desc.h"
#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"

namespace gdm {

struct Light
{
  Vec4f pos_ = Vec4f(0.f);
  Vec3f dir_ = Vec3f(0.f,1.f,0.f);
  float padding0;
  Vec4f color_ = color::Black;
  float spot_angle_ = 0.f;
  float attenuation_const_ = 1.f;
  float attenuation_linear_ = 0.09f;
  float attenuation_quadr_ = 0.032f;
  LightType type_ = LightType::POINT;
  bool  enabled_ = false;
  Vec2f padding1;

}; // struct Light

__declspec(align(64)) struct DeferredPs_PFCB
{  
  static const int v_lights_count = 16; 
  static const gfx::UboType v_type_ = gfx::EUboType::PER_FRAME; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::PX; 

  alignas(16) Vec4f global_ambient_ = Vec4f(0.2f);
  alignas(16) Vec4f camera_pos_ = Vec4f(0.f);
  alignas(16) std::array<Light, v_lights_count> lights_ = {};

}; // struct DeferredPs_PFCB

struct DeferredPassData
{
  api::Buffer* pfcb_staging_ps_;
  api::Buffer* pfcb_uniform_ps_;
  api::BufferBarrier* pfcb_to_write_barrier_;
  api::BufferBarrier* pfcb_to_read_barrier_;
  api::Image* depth_image_;
  api::ImageView* depth_image_view_;
  api::Framebuffer* fb_;
  api::DescriptorSet* descriptor_set_;
  api::ImageBarrier* present_to_read_barrier_;
  api::ImageBarrier* present_to_write_barrier_;

  DeferredPs_PFCB pfcb_data_ps_ = {};
};

struct DeferredPass
{
  DeferredPass(int frames_count, api::Renderer& ctx);
  ~DeferredPass();

public:
  void Update(uint curr_frame, const CameraEul& camera, const std::vector<ModelLight>& lamps, const std::vector<ModelLight>& flashlights);
  void Render(api::CommandList& cmd, uint frame_num);

public:
  void CreateUniforms(api::CommandList& cmd, uint frame_num);
  void CreateImages(api::CommandList& cmd);
  void CreateFramebuffer();
  void CreateRenderPass();
  void CreatePipeline(const api::ImageViews& gbuffer_image_views_);
  
public:
  void CleanupInternals();
  void CleanupPipeline();

private:
  api::Renderer* ctx_ = nullptr;
  api::Device* device_ = nullptr;
  api::Sampler* sampler_ = nullptr;
  api::RenderPass* pass_ = nullptr;
  api::Pipeline* pipeline_ = nullptr;
  
  std::vector<DeferredPassData> data_;
};

} // namespace gdm

#endif // GFX_DEFERRED_DESC_H