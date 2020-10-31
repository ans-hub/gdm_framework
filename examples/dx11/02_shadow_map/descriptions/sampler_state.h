// *************************************************************
// File:    desc/sampler_state.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_SAMPLER_DESC_H
#define SH_SAMPLER_DESC_H

#include <render/defines.h>
#include <render/sampler_state.h>

namespace gdm {

struct StdSamplerState : public SamplerState
{
  StdSamplerState() : SamplerState({
    gfx::ETextureAddressMode::WRAP,
    color::Black,
    gfx::ECompareFunc::ALWAYS})
  { }
};

} // namespace gdm

#endif // SH_SAMPLER_DESC_H
