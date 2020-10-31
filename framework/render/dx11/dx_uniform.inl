// *************************************************************
// File:    dx_uniform.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_uniform.h"

template<class T>
inline gdm::DxUniform<T>::DxUniform(ID3D11Device* device, const T& data)
  : size_{sizeof(T)}
  , data_{data}
  , buffer_{nullptr}
{
  ASSERT(device);
  ASSERTF(size_ % 16 == 0, "Invalid alignment of const buffer struct - should be 16");

  D3D11_BUFFER_DESC desc;
  memset(&desc, 0, sizeof(D3D11_BUFFER_DESC));

  desc.ByteWidth = size_;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.CPUAccessFlags = 0;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

  HRESULT hr = device->CreateBuffer(&desc, nullptr, &buffer_);
  ASSERTF(hr == S_OK, "Couldn't create const buffer - %d", hr);
}

template<class T>
inline gdm::DxUniform<T>::~DxUniform()
{
  d3d_utils::Release(buffer_);
}

template<class T>
inline void gdm::DxUniform<T>::UploadToVS(std::size_t slot_num, ID3D11DeviceContext* ctx)
{
  ASSERT(buffer_);
  ctx->UpdateSubresource(buffer_, 0, nullptr, static_cast<void*>(&data_), 0, 0);
  ctx->VSSetConstantBuffers(static_cast<UINT>(slot_num), 1, &buffer_);
}

template<class T>
inline void gdm::DxUniform<T>::UploadToPS(std::size_t slot_num, ID3D11DeviceContext* ctx)
{
  ASSERT(buffer_);
  ctx->UpdateSubresource(buffer_, 0, nullptr, static_cast<void*>(&data_), 0, 0);
  ctx->PSSetConstantBuffers(static_cast<UINT>(slot_num), 1, &buffer_);
}
