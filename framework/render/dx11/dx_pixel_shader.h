// *************************************************************
// File:    dx_pixel_shader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_PX_SHADER_H
#define AH_DX_PX_SHADER_H

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <string>
#include <memory>
#include <filesystem>

#include <system/assert_utils.h>
#include <system/string_utils.h>
#include "render/sampler_state.h"
#include "math/vector4.h"

#define DEBUG_LEVEL 1

namespace gdm::dx {

struct PixelShader
{
  PixelShader(const char* fname, const SamplerState& sampler, ID3D11Device* device, void* /*compiler_dummy*/);
  ~PixelShader();

public:
  void LoadCompiledShader(ID3DBlob* sb, ID3D11Device* device);
  void CreateSamplerState(ID3DBlob* sb, const SamplerState& sampler, ID3D11Device* device);

public:
  ID3D11PixelShader* ptr_;
  ID3D11SamplerState* sampler_state_;

}; // struct PixelShader

} // namespace gdm::dx

#endif // AH_DX_PX_SHADER_H