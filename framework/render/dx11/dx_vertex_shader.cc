// *************************************************************
// File:    dx_vertex_shader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_vertex_shader.h"

#define DEBUG_LEVEL 1

#include "dx_shader_loader.h"

namespace gdm {

// --public

dx::VertexShader::VertexShader(const char* fname, const InputLayout& layout, ID3D11Device* device, void*)
{
  DxShaderLoader loader(fname, device, gfx::EShaderType::VX);
  ID3DBlob* sb = loader.GetBlob();

  LoadCompiledShader(sb, device);
  CreateInputLayout(sb, layout, device);
}

dx::VertexShader::~VertexShader()
{
  d3d_utils::Release(static_cast<ID3D11VertexShader*>(ptr_));
  d3d_utils::Release(input_layout_);
}

// --private

void dx::VertexShader::LoadCompiledShader(ID3DBlob* sb, ID3D11Device* device)
{
  HRESULT hr = device->CreateVertexShader(sb->GetBufferPointer(), sb->GetBufferSize(), nullptr,
               &(ptr_));
  ASSERTF(hr == 0, "Failed to create compiled vertex shader from file - %d", hr);
}

void dx::VertexShader::CreateInputLayout(ID3DBlob* sb, const InputLayout& layout, ID3D11Device* device)
{
  std::vector<D3D11_INPUT_ELEMENT_DESC> vx_layout;

  for (const auto &elem : layout.GetData())
  {
    const char* name = elem.name_;
    DXGI_FORMAT type = static_cast<DXGI_FORMAT>(elem.type_);
    unsigned int offset = elem.offset_;
    D3D11_INPUT_ELEMENT_DESC desc {name, 0, type, 0, offset, D3D11_INPUT_PER_VERTEX_DATA, 0};
    vx_layout.emplace_back(desc);
  }
  ASSERTF(!vx_layout.empty(), "Input layout is empty");

  HRESULT hr = device->CreateInputLayout(vx_layout.data(), static_cast<UINT>(vx_layout.size()),
                                         sb->GetBufferPointer(), sb->GetBufferSize(),
                                         &input_layout_);
  ASSERTF(hr == S_OK, "Couldn't create input layout - %d", hr);
  vertex_size_ = layout.GetSize();
}

} // namespace gdm