// *************************************************************
// File:    dx_defines.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_DEFINES_H
#define AH_DX_DEFINES_H

#include <d3d11.h>

using uint = unsigned int;

namespace gdm::gfx {

enum EFrontFace : uint
{
  CLOCKWISE = 0,
  COUNTER_CLOCKWISE = 1

}; // enum EFrontFace

using FrontFace = uint;

enum ETextureAddressMode : uint
{ 
  WRAP = D3D11_TEXTURE_ADDRESS_WRAP,
  BORDER = D3D11_TEXTURE_ADDRESS_BORDER,
  CLAMP = D3D11_TEXTURE_ADDRESS_CLAMP

}; // enum ETextureAddressMode

using TextureAddressMode = uint;

enum ECompareFunc : uint
{ 
  ALWAYS = D3D11_COMPARISON_ALWAYS

}; // enum ECompareFunc

using CompareFunc = uint;

enum ECullMode : uint
{
  NONE = D3D11_CULL_NONE,
  BACK_FACE = D3D11_CULL_BACK,
  FRONT_FACE = D3D11_CULL_FRONT

}; // enum ECullMode

using CullMode = uint;

enum EFillMode : uint
{
  SOLID = D3D11_FILL_SOLID,
  WIREFRAME = D3D11_FILL_WIREFRAME

}; // enum EFillMode

using CullMode = uint;

enum EFormatType : uint
{
  F1 = DXGI_FORMAT_R32_FLOAT,
  F2 = DXGI_FORMAT_R32G32_FLOAT,
  F3 = DXGI_FORMAT_R32G32B32_FLOAT,
  F4 = DXGI_FORMAT_R32G32B32A32_FLOAT,
  UNORM4 = DXGI_FORMAT_R8G8B8A8_UNORM,
  D24 = DXGI_FORMAT_D24_UNORM_S8_UINT
};

using FormatType = uint;

enum EUsageType : uint
{
  DEFAULT = D3D11_USAGE_DEFAULT,
  IMMUTABLE = D3D11_USAGE_IMMUTABLE
};

using UsageType = uint;

enum ESrvDimension : uint
{
  SRV_TEX2D = D3D11_SRV_DIMENSION_TEXTURE2D
};

using SrvDimension = uint;

enum ERtvDimension : uint
{
  RTV_TEX2D = D3D11_RTV_DIMENSION_TEXTURE2D
};

using RtvDimension = uint;

enum EDsvDimension : uint
{
  DSV_TEX2D = D3D11_DSV_DIMENSION_TEXTURE2D
};

using DsvDimension = uint;

enum EBindType : uint
{
  RTV = D3D11_BIND_RENDER_TARGET,
  SRV = D3D11_BIND_SHADER_RESOURCE,
  DSV = D3D11_BIND_DEPTH_STENCIL
};

using BindType = uint;

enum EPrimitiveType : uint
{
  LINE_LIST = D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
  LINE_STRIP = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,
  TRIANGLE_LIST = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST

}; // enum PrimitiveType

using PrimitiveType = uint;

enum EPassFlags : uint
{
  RS_STATE = 1 << 1,
  RS_VIEWPORT = 1 << 2,
  CLEAR_SHADER_RES = 1 << 3,
  NULL_VBUFF = 1 << 4,
  NULL_IBUFF = 1 << 5,
  DONT_CLEAR_RTV = 1 << 6,
  DONT_CLEAR_DSV = 1 << 7

}; // enum EPassFlags

using PassFlags = uint;

} // namespace gdm::gfx

#endif // AH_DX_DEFINES_H
