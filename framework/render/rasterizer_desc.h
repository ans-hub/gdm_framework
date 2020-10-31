// *************************************************************
// File:    rasterizer_desc.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_RASTERIZER_DESC_H
#define AH_RASTERIZER_DESC_H

#include "defines.h"

namespace gdm {

struct RasterizerDesc
{
  gfx::ECullMode cull_ = gfx::BACK_FACE;
  gfx::EFillMode fill_ = gfx::SOLID;

}; // struct RasterizerDesc

} // namespace gdm

#endif // AH_RASTERIZER_DESC_H
