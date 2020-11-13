// *************************************************************
// File:    gbuffer_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_GBUFFER_DESC_H
#define GFX_GBUFFER_DESC_H

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/shader.h"

#include "math/matrix.h"
#include "math/vector4.h"

#include "desc/sampler_desc.h"
#include "desc/rasterizer_desc.h"
#include "desc/input_layout.h"

namespace gdm {

struct alignas(64) GbufferVs_PFCB
{
  alignas(16) Mat4f u_view_proj_;
  alignas(16) Vec3f u_cam_pos_;
  float dummy;

  static const gfx::UboType v_type_ = gfx::EUboType::PER_FRAME; 

}; // struct GbufferVs_PFCB

struct alignas(64) GbufferVs_POCB
{
  alignas(16) Mat4f u_model_;
  alignas(16) Vec4f u_color_;
  alignas(16) unsigned int u_material_index_;

  static const gfx::UboType v_type_ = gfx::EUboType::PER_OBJECT; 
  
}; // struct GbufferVs_POCB

struct GbufferPassData
{
  api::Buffer* pfcb_uniform_;
  api::Buffer* pfcb_staging_;
  api::BufferBarrier* pfcb_to_write_barrier_;
  api::BufferBarrier* pfcb_to_read_barrier_;
  api::Buffer* pocb_uniform_;
  api::Buffer* pocb_staging_;
  api::BufferBarrier* pocb_to_write_barrier_;
  api::BufferBarrier* pocb_to_read_barrier_;
  api::Sampler* sampler_;
  std::vector<api::Image2D*> images_; // pos, diff, norm
  std::vector<api::ImageView*> image_views_;  // pos, diff, norm
  std::vector<api::ImageBarrier*> image_barriers_to_read_;
  std::vector<api::ImageBarrier*> image_barriers_to_write_;
  api::Framebuffer* fb_;
  api::DescriptorSetLayout* descriptor_set_layout_;
  api::DescriptorSet* descriptor_set_;  

  GbufferVs_PFCB pfcb_data_;
  std::vector<GbufferVs_POCB> pocb_data_;
};

struct GbufferPass
{
  GbufferPass(api::Renderer& rdr, uint max_objects)
    : rdr_{&rdr}
    , device_{&rdr.GetDevice()}
    , data_()
  {
    data_.pocb_data_.resize(max_objects);
  }

  api::Renderer* rdr_ = nullptr;
  api::Device* device_ = nullptr;
  api::Sampler* sampler_ = nullptr;
  api::RenderPass* pass_ = nullptr;
  api::Pipeline* pipeline_ = nullptr;

  GbufferPassData data_;

  void CreateImages(api::CommandList& cmd);
  void CreateFramebuffer();
  void CreateRenderPass();
  void CreateDescriptorSet(api::ImageViews& diffuse, api::ImageViews& specular, api::ImageViews& normals);
  void CreatePipeline();
};

} // namespace gdm

#endif // GFX_GBUFFER_DESC_H