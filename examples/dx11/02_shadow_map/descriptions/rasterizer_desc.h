// *************************************************************
// File:    desc/rasterizer_desc.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_RASTERIZER_DESC_H
#define SH_RASTERIZER_DESC_H

#include <render/defines.h>
#include <render/rasterizer_desc.h>

namespace gdm {
struct RasterizerMain : public RasterizerDesc
{
  RasterizerMain() : RasterizerDesc()
  {
    cull_ = gfx::ECullMode::BACK_FACE;
    fill_ = gfx::EFillMode::SOLID;    
  }

}; // struct RasterizerMain

struct RasterizerShadowMap : public RasterizerDesc
{
  RasterizerShadowMap() : RasterizerDesc()
  {
    cull_ = gfx::ECullMode::FRONT_FACE;
    fill_ = gfx::EFillMode::SOLID;
  }

}; // struct RasterizerShadowMap

struct RasterizerWireframe : public RasterizerDesc
{
  RasterizerWireframe() : RasterizerDesc()
  {
    cull_ = gfx::ECullMode::NONE;
    fill_ = gfx::EFillMode::WIREFRAME;
  }

}; // struct RasterizerWireframe

} // namespace gdm

#endif // SH_RASTERIZER_DESC_H
