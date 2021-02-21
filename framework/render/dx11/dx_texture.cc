// *************************************************************
// File:    dx_texture.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_texture.h"

#include <render/defines.h>
#include <system/bits_utils.h>

// --public

gdm::DxTexture::DxTexture(ID3D11Device* device, int width, int height, TextureDesc desc /* {} */)
  : texture_{nullptr}
  , render_target_view_{nullptr}
  , shader_resource_view_{nullptr}
  , depth_stencil_view_{nullptr}
{
  CreateTexture(device, width, height, desc);
  CreateViews(device, desc);
}

gdm::DxTexture::DxTexture(ID3D11Device* device, AbstractImage& img, TextureDesc desc /* {} */)
  : texture_{nullptr}
  , render_target_view_{nullptr}
  , shader_resource_view_{nullptr}
  , depth_stencil_view_{nullptr}
{
  CreateTexture(device, img, desc);
  CreateViews(device, desc);
}

void gdm::DxTexture::Release()
{
  d3d_utils::Release(shader_resource_view_);
  d3d_utils::Release(texture_);
  d3d_utils::Release(render_target_view_);
  d3d_utils::Release(depth_stencil_view_);
}

gdm::DxTexture::DxTexture(DxTexture&& tex)
  : shader_resource_view_{tex.shader_resource_view_}
  , texture_{tex.texture_}
  , render_target_view_{tex.render_target_view_}
  , depth_stencil_view_{tex.depth_stencil_view_}
{
  tex.shader_resource_view_ = nullptr;
  tex.texture_ = nullptr;
  tex.render_target_view_ = nullptr;
  tex.depth_stencil_view_ = nullptr;
}

gdm::DxTexture& gdm::DxTexture::operator=(DxTexture&& tex)
{
  if (this != &tex)
  {
    shader_resource_view_ = tex.shader_resource_view_;
    texture_ = tex.texture_;
    render_target_view_ = tex.render_target_view_;
    depth_stencil_view_ = tex.depth_stencil_view_;
    tex.shader_resource_view_ = nullptr;
    tex.texture_ = nullptr;
    tex.render_target_view_ = nullptr;
    tex.depth_stencil_view_ = nullptr;
  }
  return *this;
}

// --private

void gdm::DxTexture::CreateTexture(ID3D11Device* device, int width, int height, const TextureDesc& desc)
{
  D3D11_TEXTURE2D_DESC tex_desc;
  tex_desc.Width = width;
  tex_desc.Height = height;
  tex_desc.MipLevels = 1;
  tex_desc.ArraySize = 1;
  tex_desc.Format = static_cast<DXGI_FORMAT>(desc.format_);
  tex_desc.SampleDesc.Count = 1;
  tex_desc.SampleDesc.Quality = 0;
  tex_desc.Usage = static_cast<D3D11_USAGE>(desc.usage_);
  tex_desc.BindFlags = static_cast<UINT>(desc.bind_type_);
  tex_desc.CPUAccessFlags = 0;
  tex_desc.MiscFlags = 0;

  HRESULT hr = device->CreateTexture2D(&tex_desc, nullptr, &texture_);
  ASSERTF(hr == S_OK, "Couldn't create texture - texture 2d");
}

void gdm::DxTexture::CreateTexture(ID3D11Device* device, AbstractImage& img, const TextureDesc& desc)
{
  D3D11_TEXTURE2D_DESC tex_desc;
  tex_desc.Width = img.GetWidth();
  tex_desc.Height = img.GetHeight();
  tex_desc.MipLevels = 1;
  tex_desc.ArraySize = 1;
  tex_desc.Format = static_cast<DXGI_FORMAT>(desc.format_);
  tex_desc.SampleDesc.Count = 1;
  tex_desc.SampleDesc.Quality = 0;
  tex_desc.Usage = static_cast<D3D11_USAGE>(desc.usage_);
  tex_desc.BindFlags = static_cast<UINT>(desc.bind_type_);
  tex_desc.CPUAccessFlags = 0;
  tex_desc.MiscFlags = 0;

  ENSUREF(img.GetDepth() != 24, "DirectX not support %d bit tex images", img.GetDepth());

  D3D11_SUBRESOURCE_DATA init_data;
  init_data.pSysMem = img.GetRaw().data();
  init_data.SysMemPitch = static_cast<UINT>((img.GetWidth() * img.GetDepth() + 7) / 8);
  init_data.SysMemSlicePitch = static_cast<UINT>(init_data.SysMemPitch * img.GetHeight());  // just for 3d texs?

  HRESULT hr = device->CreateTexture2D(&tex_desc, &init_data, &texture_);
  ASSERTF(hr == S_OK, "Couldn't create texture - texture 2d");
}

void gdm::DxTexture::CreateViews(ID3D11Device* device, const TextureDesc& desc)
{
  if (bits::HasFlag(desc.bind_type_, gfx::SRV))
    CreateShaderResourceView(device, desc);
  if (bits::HasFlag(desc.bind_type_, gfx::RTV))
    CreateRenderTargetView(device, desc);
  if (bits::HasFlag(desc.bind_type_, gfx::DSV))
    CreateDepthStencilView(device, desc);
}

void gdm::DxTexture::CreateShaderResourceView(ID3D11Device* device, const TextureDesc& desc)
{
  D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
  memset(&srv_desc, 0, sizeof(srv_desc));
  srv_desc.Format = static_cast<DXGI_FORMAT>(desc.format_);
  srv_desc.ViewDimension = static_cast<D3D11_SRV_DIMENSION>(desc.srv_dimension_);
  srv_desc.Texture2D.MipLevels = 1;

  HRESULT hr = device->CreateShaderResourceView(texture_, &srv_desc, &shader_resource_view_);
  ASSERTF(hr == S_OK, "Couldn't create texture - shader resource view");
}

void gdm::DxTexture::CreateRenderTargetView(ID3D11Device* device, const TextureDesc& desc)
{
  D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
  memset(&rtv_desc, 0, sizeof(rtv_desc));
  rtv_desc.Format = static_cast<DXGI_FORMAT>(desc.format_);
  rtv_desc.ViewDimension = static_cast<D3D11_RTV_DIMENSION>(desc.rtv_dimension_);
  rtv_desc.Texture2D.MipSlice = 0;

  HRESULT hr = device->CreateRenderTargetView(texture_, &rtv_desc, &render_target_view_);
  ASSERTF(hr == S_OK, "Couldn't create texture - render target view");
}

void gdm::DxTexture::CreateDepthStencilView(ID3D11Device* device, const TextureDesc& desc)
{
  D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
  memset(&dsv_desc, 0, sizeof(dsv_desc));
  dsv_desc.Format = static_cast<DXGI_FORMAT>(desc.format_);
  dsv_desc.ViewDimension = static_cast<D3D11_DSV_DIMENSION>(desc.dsv_dimension_);
  dsv_desc.Texture2D.MipSlice = 0;

  HRESULT hr = device->CreateDepthStencilView(texture_, &dsv_desc, &depth_stencil_view_);
  ASSERTF(hr == S_OK, "Couldn't create texture - depth stencil view");
}
