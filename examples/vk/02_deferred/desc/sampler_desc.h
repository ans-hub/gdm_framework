// *************************************************************
// File:    desc/sampler_state.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_STD_SAMPLER_DESC_H
#define SH_STD_SAMPLER_DESC_H

#include <render/defines.h>
#include <render/sampler_state.h>
#include <render/colors.h>

namespace gdm {

struct StdSamplerState : public SamplerState
{
  StdSamplerState() : SamplerState({
    gfx::ETextureAddressMode::WRAP,
    color::Green,
    gfx::ECompareFunc::ALWAYS})
  { }
};

} // namespace gdm

#endif // SH_STD_SAMPLER_DESC_H
