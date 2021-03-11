// *************************************************************
// File:    text_pass.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_TEXT_DESC_H
#define GFX_TEXT_DESC_H

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "render/camera_eul.h"

#include "math/matrix.h"
#include "math/vector4.h"

#include "system/font.h"

#include "render/desc/sampler_desc.h"
#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"

#include "scene/debug_draw.h"

namespace gdm {

__declspec(align(64)) struct TextVs_PFCB
{  
  static const gfx::UboType v_type_ = gfx::EUboType::PER_FRAME; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::VX; 

  alignas(16) Mat4f view_proj_ = Mat4f(1.f);

}; // struct TextVs_PFCB

struct TextPassData
{
  TextPassData(api::Renderer& rdr)
    : rdr_{&rdr}
    , device_{device_}
  { }

  api::Renderer* rdr_;
  api::Device* device_;
  api::Buffer* pfcb_uniform_vs_;
  api::Framebuffer* fb_;
  api::Buffer* vertex_buffer_;
  api::DescriptorSetLayout* descriptor_set_layout_;
  api::DescriptorSet* descriptor_set_;
  api::ImageBarrier* present_to_read_barrier_;
  api::ImageBarrier* present_to_write_barrier_;

  TextVs_PFCB pfcb_data_vs_ = {};
};

class TextPass
{
  constexpr static const uint v_max_string_ = 1024;
  constexpr static const uint v_vxs_per_char_ = 4;

  api::Renderer* rdr_ = nullptr;
  api::Device* device_ = nullptr;
  api::Sampler* sampler_ = nullptr;
  api::RenderPass* pass_ = nullptr;
  api::Pipeline* pipeline_ = nullptr;

  std::vector<TextPassData> data_ = {};
  const Font* font_ = nullptr;
  api::ImageView* font_texture_ = nullptr;
  uint characters_count_ = 0;

public:
  TextPass(int frame_count, api::Renderer& rdr)
    : rdr_{&rdr}
    , device_{&rdr.GetDevice()}
    , data_(frame_count, rdr)
    , font_{nullptr}
    , font_texture_{nullptr}
    , characters_count_{0}
  { }

  void BindFont(const Font* font, api::ImageView* font_texture);
  void BindFramebuffer(api::Framebuffer* fb, uint frame_num);
  
  void CreateUniforms(api::CommandList& cmd, uint frame_num);
  void CreateVertexBuffer(api::CommandList& cmd, uint frame_num);
  void CreateRenderPass();
  void CreatePipeline();

  void UpdateUniformsData(uint curr_frame, const CameraEul& camera);
  void UpdateUniforms(api::CommandList& cmd, uint frame_num);
  void UpdateVertexData(api::CommandList& cmd, uint curr_frame, const std::vector<TextData>& text_data);
  void Draw(api::CommandList& cmd, uint curr_frame);
};

} // namespace gdm

#endif // GFX_TEXT_PASS_H