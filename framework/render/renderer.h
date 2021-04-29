
// *************************************************************
// File:    renderer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_RENDERER_H
#define AH_RENDERER_H

#if defined(GFX_DX_API)
#include "dx11/dx_renderer.h"
#elif defined (GFX_VK_API)
#include "vk/vk_renderer.h"
#else
#include "ogl/gl_renderer.h"
#endif

namespace gdm {

namespace gfxapi {
#if defined(GFX_DX_API)
  using RendererBase = DxRenderer;
#elif defined (GFX_VK_API)
  using RendererBase = ::gdm::vk::Renderer;
#else
  using RendererBase = OglRenderer;
#endif
}

struct Renderer : public gfxapi::RendererBase
{
  template<class...Args>
  Renderer(Args&&...args)
    : gfxapi::RendererBase(std::forward<Args>(args)...)
  { }
};

} // namespace gdm

#endif // AH_RENDERER_H
