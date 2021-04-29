// *************************************************************
// File:    vertex_shader.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_VX_SHADER_DESC_H
#define AH_VX_SHADER_DESC_H

#include <vector>

#include "defines.h"
#include "renderer.h"

#include <render/input_layout.h>

#if defined (GFX_DX_API)
#include "dx11/dx_vertex_shader.h"
#elif defined (GFX_VK_API)
#include "vk/vk_vertex_shader.h"
#else
#include "gl/gl_vertex_shader.h"
#endif

namespace gdm {

struct VertexShader
{
  VertexShader(const char* fname, const InputLayout& layout, Renderer& ctx)
    : input_layout_{layout}
    , impl_{fname, layout, ctx.GetDevice(), ctx.GetCompiler()}
  { }
  auto GetImpl() -> api::VertexShader& { return impl_; }

private:
  InputLayout input_layout_;
  api::VertexShader impl_;

}; // struct VertexShader

} // namespace gdm

#endif // AH_VX_SHADER_DESC_H
