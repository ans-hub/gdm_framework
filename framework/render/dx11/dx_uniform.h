// *************************************************************
// File:    dx_uniform.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_UNIFORM_H
#define AH_DX_UNIFORM_H

#include <initializer_list>
#include <memory>

#include <d3d11.h>

namespace gdm {

template<class T>
struct DxUniform
{
  DxUniform(ID3D11Device* device, const T& data = T{});
  ~DxUniform();

  void UploadToVS(std::size_t slot_num, ID3D11DeviceContext* ctx);
  void UploadToPS(std::size_t slot_num, ID3D11DeviceContext* ctx);

  std::size_t GetSize() const { return size_; }
  T& GetData() { return data_; }
  void SetData(std::initializer_list<T> list) { data_ = T{list}; }
  void SetData(void* ptr) { std::memcpy(&data_, ptr, sizeof(T)); }
  void SetData(const T& data) { data_ = data; }
  T* operator->() { return &data_; }
  const T* operator->() const { return data_; }

private:
  UINT size_;
  T data_;
  ID3D11Buffer* buffer_;

}; // struct DxUniform

} // namespace gdm

#include "dx_uniform.inl"

#endif // AH_DX_UNIFORM_H