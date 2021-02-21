// *************************************************************
// File:    sampler_desc.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_STD_SAMPLER_DESC_H
#define SH_STD_SAMPLER_DESC_H

#include <render/defines.h>
#include <render/colors.h>

namespace gdm {

struct SamplerDesc
{
  gfx::ETextureAddressMode address_ = gfx::WRAP;
  Vec4f border_color_ = {0.f, 0.f, 0.f, 0.f};
  gfx::ECompareFunc comparsion_ = gfx::ALWAYS;

}; // struct SamplerDesc

struct StdSamplerDesc : public SamplerDesc
{
  StdSamplerDesc() : SamplerDesc({
    gfx::ETextureAddressMode::WRAP,
    color::Green,
    gfx::ECompareFunc::ALWAYS})
  { }
};

} // namespace gdm

#endif // SH_STD_SAMPLER_DESC_H
