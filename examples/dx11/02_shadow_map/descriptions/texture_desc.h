// *************************************************************
// File:    desc/texture_desc.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_TEXTURE_DESC_H
#define SH_TEXTURE_DESC_H

#include <render/defines.h>
#include <render/texture_desc.h>

namespace gdm {

struct TextureDescMain : public TextureDesc
{
  TextureDescMain() : TextureDesc()
  {
    format_ = gfx::EFormatType::F4;
    usage_ = gfx::EUsageType::DEFAULT;
    srv_dimension_ = gfx::ESrvDimension::SRV_TEX2D;
    rtv_dimension_ = gfx::ERtvDimension::RTV_TEX2D;
    bind_type_ = gfx::RTV | gfx::SRV;
  }

}; // struct TextureDescMain

struct TextureDescDepth : public TextureDesc
{
  TextureDescDepth() : TextureDesc()
  {
    format_ = gfx::EFormatType::D24;
    usage_ = gfx::EUsageType::DEFAULT;
    dsv_dimension_ = gfx::EDsvDimension::DSV_TEX2D;
    bind_type_ = gfx::DSV;
  }

}; // struct TextureDescDepth

} // namespace gdm

#endif // SH_TEXTURE_DESC_H
