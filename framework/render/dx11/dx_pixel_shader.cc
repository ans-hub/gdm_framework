// *************************************************************
// File:    dx_pixel_shader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_pixel_shader.h"

#include "dx_shader_loader.h"

namespace gdm {
  
// --public

dx::PixelShader::PixelShader(const char* fname, const SamplerState& sampler, ID3D11Device* device, void*)
{
  DxShaderLoader loader(fname, device, gfx::EShaderType::PX);
  ID3DBlob* sb = loader.GetBlob();

  LoadCompiledShader(sb, device);
  CreateSamplerState(sb, sampler, device);
}

dx::PixelShader::~PixelShader()
{
  d3d_utils::Release(static_cast<ID3D11PixelShader*>(ptr_));
  d3d_utils::Release(sampler_state_);
}

// --private

void dx::PixelShader::LoadCompiledShader(ID3DBlob* sb, ID3D11Device* device)
{
  HRESULT hr = device->CreatePixelShader(sb->GetBufferPointer(), sb->GetBufferSize(), nullptr,
               &(static_cast<ID3D11PixelShader*>(ptr_)));
  ASSERTF(hr == 0, "Failed to create compiled pixes shader from file - %d", hr);
}

void gdm::dx::PixelShader::CreateSamplerState(ID3DBlob* sb, const SamplerState& sampler, ID3D11Device* device)
{
  D3D11_SAMPLER_DESC sampler_desc;

  sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sampler_desc.AddressU = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(sampler.address_);
  sampler_desc.AddressV = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(sampler.address_);
  sampler_desc.AddressW = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(sampler.address_);
  sampler_desc.MipLODBias = 0.0f;
  sampler_desc.MaxAnisotropy = 1;
  sampler_desc.ComparisonFunc = static_cast<D3D11_COMPARISON_FUNC>(sampler.comparsion_);
  sampler_desc.BorderColor[0] = sampler.border_color_.x;
  sampler_desc.BorderColor[1] = sampler.border_color_.y;
  sampler_desc.BorderColor[2] = sampler.border_color_.z;
  sampler_desc.BorderColor[3] = sampler.border_color_.w;
  sampler_desc.MinLOD = 0;
  sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

  HRESULT hr = device->CreateSamplerState(&sampler_desc, &sampler_state_);
  ASSERTF(hr == S_OK, "Couldn't create sampler state - %d", hr);
}

} // namespace gdm