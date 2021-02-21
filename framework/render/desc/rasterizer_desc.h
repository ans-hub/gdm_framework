// *************************************************************
// File:    desc/sampler_state.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_STD_RAST_DESC_H
#define SH_STD_RAST_DESC_H

#include <render/defines.h>
#include <render/desc/rasterizer_desc.h>

namespace gdm {

struct RasterizerDesc
{
  gfx::ECullMode cull_ = gfx::BACK_FACE;
  gfx::EFillMode fill_ = gfx::SOLID;
  gfx::EFrontFace front_face_ = gfx::CLOCKWISE;

}; // struct RasterizerDesc

struct StdRasterizerDesc : public RasterizerDesc
{
  StdRasterizerDesc() : RasterizerDesc({
    gfx::ECullMode::BACK_FACE,
    gfx::EFillMode::SOLID,
    gfx::EFrontFace::CLOCKWISE})
  { }
};

struct ReverseRasterizerDesc : public RasterizerDesc
{
  ReverseRasterizerDesc() : RasterizerDesc({
    gfx::ECullMode::FRONT_FACE,
    gfx::EFillMode::SOLID,
    gfx::EFrontFace::COUNTER_CLOCKWISE})
  { }
};

struct WireRasterizerDesc : public RasterizerDesc
{
  WireRasterizerDesc() : RasterizerDesc({
    gfx::ECullMode::NONE,
    gfx::EFillMode::WIREFRAME,
    gfx::EFrontFace::CLOCKWISE})
  { }
};

} // namespace gdm

#endif // SH_STD_RAST_DESC_H
