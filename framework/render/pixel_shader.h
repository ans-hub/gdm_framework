// *************************************************************
// File:    pixel_shader.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_PX_SHADER_DESC_H
#define AH_PX_SHADER_DESC_H

#include <vector>

#include <memory/defines.h>

#include "defines.h"
#include "renderer.h"

#include <render/desc/sampler_desc.h>

#if defined (GFX_DX_API)
#include "dx11/dx_pixel_shader.h"
#elif defined (GFX_VK_API)
#include "vk/vk_pixel_shader.h"
#else
#include "gl/gl_pixel_shader.h"
#endif

namespace gdm {

struct PixelShader
{
  PixelShader(const char* fname, const SamplerDesc& sampler, Renderer& ctx)
    : sampler_state_{sampler}
    , impl_{fname, sampler, ctx.GetDevice(), ctx.GetCompiler()}
  { }
  auto GetImpl() -> api::PixelShader& { return impl_; }

private:
  SamplerDesc sampler_state_;
  api::PixelShader impl_;

}; // struct PixelShader

} // namespace gdm

#endif // AH_PX_SHADER_DESC_H
