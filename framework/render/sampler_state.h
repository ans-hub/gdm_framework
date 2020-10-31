// *************************************************************
// File:    sampler_state.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_SAMPLER_DESC_H
#define AH_SAMPLER_DESC_H

#include "render/defines.h"
#include "math/vector4.h"

namespace gdm {

struct SamplerState
{
  gfx::ETextureAddressMode address_ = gfx::CLAMP;
  Vec4f border_color_ = {0.f, 0.f, 0.f, 0.f};
  gfx::ECompareFunc comparsion_ = gfx::ALWAYS;

}; // struct SamplerState

} // namespace gdm

#endif // AH_SAMPLER_DESC_H
