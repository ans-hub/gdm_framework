// *************************************************************
// File:    texture_desc.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_TEXTURE_DESC_H
#define AH_TEXTURE_DESC_H

#include "math/vector2.h"
#include "math/vector3.h"

#include "defines.h"

namespace gdm {

struct TextureDesc
{
  gfx::EFormatType format_ = gfx::EFormatType::UNORM4;
  gfx::EUsageType usage_ = gfx::EUsageType::DEFAULT;
  gfx::ESrvDimension srv_dimension_ =  gfx::ESrvDimension::SRV_TEX2D;
  gfx::ERtvDimension rtv_dimension_ =  gfx::ERtvDimension::RTV_TEX2D;
  gfx::EDsvDimension dsv_dimension_ =  gfx::EDsvDimension::DSV_TEX2D;
  gfx::BindType bind_type_ = gfx::EBindType::RTV | gfx::EBindType::SRV;

}; // struct TextureDesc

} // namespace gdm

#endif // AH_TEXTURE_DESC_H
