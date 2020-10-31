// *************************************************************
// File:    dx_texture.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_TEXTURE_H
#define AH_DX_TEXTURE_H

#include <d3d11.h>

#include "dx_utils.h"

#include <system/assert_utils.h>
#include <data/abstract_image.h>

#include <render/texture_desc.h>

namespace gdm {

struct DxRenderer;

struct DxTexture
{
  friend struct DxRenderer;
  
  DxTexture() =default;
  DxTexture(ID3D11Device* device, AbstractImage& img, TextureDesc desc = {});
  DxTexture(ID3D11Device* device, int width, int height, TextureDesc desc = {});
  ~DxTexture() { Release(); }

  DxTexture(const DxTexture& tex) =delete;
  DxTexture& operator=(const DxTexture& tex) =delete;
  DxTexture(DxTexture&& tex);
  DxTexture& operator=(DxTexture&& tex);

  ID3D11RenderTargetView* GetRenderTargetView() { return render_target_view_; }
  ID3D11ShaderResourceView* GetShaderResourceView() { return shader_resource_view_; }
  ID3D11DepthStencilView* GetDepthStencilView() { return depth_stencil_view_; }

private:
  void CreateTexture(ID3D11Device* device, int width, int height, const TextureDesc& desc);
  void CreateTexture(ID3D11Device* device, AbstractImage& img, const TextureDesc& desc);
  void CreateViews(ID3D11Device* device, const TextureDesc& desc);
  void CreateShaderResourceView(ID3D11Device* device, const TextureDesc& desc);
  void CreateRenderTargetView(ID3D11Device* device, const TextureDesc& desc);
  void CreateDepthStencilView(ID3D11Device* device, const TextureDesc& desc);

private:
  ID3D11Texture2D* texture_;
  ID3D11RenderTargetView* render_target_view_;
  ID3D11ShaderResourceView* shader_resource_view_;
  ID3D11DepthStencilView* depth_stencil_view_;

  virtual void Release();

}; // struct DxTexture

} // namespace gdm

#endif // AH_DX_TEXTURE_H
