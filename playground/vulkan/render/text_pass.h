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
#include "engine/camera_eul.h"

#include "math/matrix.h"
#include "math/vector4.h"

#include "system/font.h"

#include "render/desc/sampler_desc.h"
#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"

#include "engine/debug_draw.h"

namespace gdm {

__declspec(align(64)) struct TextFs_POCB
{  
  static const gfx::UboType v_type_ = gfx::EUboType::PER_OBJECT; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::PX; 

  alignas(16) Vec4f color_ = Vec4f(1.f);

}; // struct TextFs_POCB

struct TextPassData
{
  TextPassData(api::Renderer& rdr);

  api::Renderer* rdr_;
  api::Device* device_;
  api::Buffer* pocb_uniform_fs_;
  api::Framebuffer* fb_;
  api::Buffer* vertex_buffer_;
  api::DescriptorSet* descriptor_set_;
  api::ImageBarrier* present_to_read_barrier_;
  api::ImageBarrier* present_to_write_barrier_;
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
  const api::ImageView* font_texture_ = nullptr;
  std::vector<std::pair<uint, Vec4f>> strings_ = {};

public:
  TextPass(int frame_count, api::Renderer& rdr);
  ~TextPass();

  void CleanupInternals();
  void CleanupPipeline();

  void BindFont(const Font* font, const api::ImageView* font_texture);
  
  void CreateUniforms(api::CommandList& cmd, uint frame_num);
  void CreateVertexBuffer(api::CommandList& cmd, uint frame_num);
  void CreateBarriers(api::CommandList& cmd);
  void CreateFramebuffer();
  void CreateRenderPass();
  void CreatePipeline();

  void UpdateVertexData(api::CommandList& cmd, uint curr_frame, const std::vector<TextData>& text_data);
  void Draw(api::CommandList& cmd, uint curr_frame);
  void DestroyBarriers();

  [[deprecated]] void BindFramebuffer(api::Framebuffer* fb, uint frame_num);
};

} // namespace gdm

#endif // GFX_TEXT_PASS_H