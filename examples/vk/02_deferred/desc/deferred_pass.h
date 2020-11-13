// *************************************************************
// File:    deferred_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_DEFERRED_DESC_H
#define GFX_DEFERRED_DESC_H

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/colors.h"
#include "render/shader.h"

#include "desc/sampler_desc.h"
#include "desc/rasterizer_desc.h"
#include "desc/input_layout.h"

namespace gdm {

enum LightType : int
{
  DIR, POINT 
};

__declspec(align(64)) struct Light
{
  Vec4f pos_ = Vec4f(0.f);
  Vec4f dir_ = Vec4f(0.f);
  Vec4f color_ = color::Black;
  float spot_angle_ = 0.f;
  float attenuation_const_ = 1.f;
  float attenuation_linear_ = 0.08f;
  float attenuation_quadr_ = 0.0f;
  LightType type_ = LightType::DIR;
  bool  enabled_ = false;

}; // struct Light

__declspec(align(64)) struct DeferredPs_PFCB
{
  alignas(16) Vec4f global_ambient_ = Vec4f(0.2f);
  alignas(16) Vec4f camera_pos_ = Vec4f(0.f);
  alignas(16) std::array<Light,4> lights_ = {};
  
  static const gfx::UboType v_type_ = gfx::EUboType::PER_FRAME; 

}; // struct DeferredPs_PFCB

struct DeferredPassData
{
  DeferredPassData(api::Renderer& rdr) : rdr_{&rdr}, device_{device_} { }

  api::Renderer* rdr_;
  api::Device* device_;
  api::Buffer* pfcb_uniform_;
  api::Buffer* pfcb_staging_;
  api::BufferBarrier* pfcb_to_write_barrier_;
  api::BufferBarrier* pfcb_to_read_barrier_;
  api::Image2D* depth_image_;
  api::ImageView* depth_image_view_;
  api::Framebuffer* fb_;
  api::DescriptorSet* descriptor_set_;
  api::ImageBarrier* present_to_read_barrier_;
  api::ImageBarrier* present_to_write_barrier_;

  DeferredPs_PFCB pfcb_data_ = {};
};

struct DeferredPass
{
  DeferredPass(int frame_num, api::Renderer& rdr) : rdr_{&rdr}, device_{&rdr.GetDevice()}, data_(frame_num, rdr) { }
  
  api::Renderer* rdr_ = nullptr;
  api::Device* device_ = nullptr;
  api::Sampler* sampler_ = nullptr;
  api::RenderPass* pass_ = nullptr;
  api::Pipeline* pipeline_ = nullptr;
  
  std::vector<DeferredPassData> data_;

  void CreateImages(api::CommandList& cmd);
  void CreateFramebuffer();
  void CreateRenderPass();
  void CreatePipeline(const api::ImageViews& gbuffer_image_views_);
};

} // namespace gdm

#endif // GFX_DEFERRED_DESC_H