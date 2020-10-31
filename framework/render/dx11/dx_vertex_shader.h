// *************************************************************
// File:    dx_vertex_shader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_VX_SHADER_H
#define AH_DX_VX_SHADER_H

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <string>
#include <memory>
#include <filesystem>

#include <system/assert_utils.h>
#include <system/string_utils.h>
#include <math/vector4.h>
#include <render/input_layout.h>

namespace gdm::dx {

struct VertexShader
{
  VertexShader(const char* fname, const InputLayout& layout, ID3D11Device* device, void* /*compiler_dummy*/);
  ~VertexShader();

public:
  void LoadCompiledShader(ID3DBlob* sb, ID3D11Device* device);
  void CreateInputLayout(ID3DBlob* sb, const InputLayout& layout, ID3D11Device* device);

public:
  ID3D11VertexShader* ptr_;
  ID3D11InputLayout* input_layout_;
  uint vertex_size_;

}; // struct VertexShader

} // namespace gdm::dx

#endif // AH_DX_VX_SHADER_H