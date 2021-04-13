// *************************************************************
// File:    debug_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_DEBUG_DESC_H
#define GFX_DEBUG_DESC_H

#include <map>
#include <functional>

#include "render/defines.h"
#include "render/api.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "engine/camera_eul.h"

#include "math/matrix.h"
#include "math/vector4.h"

#include "data/model_factory.h"
#include "data/material_factory.h"

#include "render/desc/sampler_desc.h"
#include "render/desc/rasterizer_desc.h"
#include "render/desc/input_layout_desc.h"

#include "engine/debug_draw.h"
#include "engine/gui_manager.h"

namespace gdm {

__declspec(align(64)) struct DebugVs_PFCB
{  
  static const gfx::UboType v_type_ = gfx::EUboType::PER_FRAME; 
  static const gfx::ShaderType v_shader_type_ = gfx::EShaderType::VX; 

  alignas(16) Mat4f view_proj_ = Mat4f(1.f);

}; // struct DebugVs_PFCB

struct DebugPassData
{
  DebugPassData(api::Renderer& rdr);

  api::Renderer* rdr_;
  api::Device* device_;
  api::Buffer* pfcb_staging_vs_;
  api::Buffer* pfcb_uniform_vs_;
  api::BufferBarrier* pfcb_to_write_barrier_;
  api::BufferBarrier* pfcb_to_read_barrier_;
  api::Framebuffer* fb_;
  api::Buffer* vertex_buffer_;
  api::ImageBarrier* present_to_read_barrier_;
  api::ImageBarrier* present_to_write_barrier_;
  api::DescriptorSet* descriptor_set_;  
  size_t vertices_count_ = 0;

  DebugVs_PFCB pfcb_data_vs_ = {};

}; // struct DebugPassData

struct DebugPass
{
  DebugPass(int frame_count, api::Renderer& rdr);
  ~DebugPass();

  void Cleanup();

  api::Renderer* rdr_ = nullptr;
  api::Device* device_ = nullptr;
  api::Sampler* sampler_ = nullptr;
  api::RenderPass* pass_ = nullptr;
  api::Pipeline* pipeline_ = nullptr;

  std::vector<DebugPassData> data_;

  void CreateUniforms(api::CommandList& cmd, uint frame_num);
  void CreateVertexBuffer(api::CommandList& cmd, uint frame_num, uint64 buffer_size);
  void CreateBarriers(api::CommandList& cmd);
  void CreateFramebuffer();
  void CreateRenderPass();
  void CreatePipeline();
  void CreateGui();
  
  void UpdateUniforms(api::CommandList& cmd, uint frame_num);
  void UpdateUniformsData(uint curr_frame, const CameraEul& camera);
  void UpdateVertexData(api::CommandList& cmd, uint curr_frame, const std::vector<DebugData>& debug_data);

  void Draw(api::CommandList& cmd, uint curr_frame, bool debug_stage_active, bool gui_stage_active, const std::vector<GuiCallback>& gui_callbacks);

private:
  void DrawPrimitives(api::CommandList& cmd, uint curr_frame);
  void DrawGui(api::CommandList& cmd, uint curr_frame);

}; // struct DebugPass

} // namespace gdm

#endif // GFX_DEBUG_DESC_H
