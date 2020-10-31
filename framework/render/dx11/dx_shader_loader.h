// *************************************************************
// File:    dx_shader_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_SHADER_LOADER_H
#define AH_DX_SHADER_LOADER_H

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#include <string>
#include <memory>
#include <filesystem>

#include <system/assert_utils.h>
#include <system/string_utils.h>
#include <math/vector4.h>

#include "render/defines.h"
#include "render/dx11/dx_utils.h"
#include "render/dx11/dx_defines.h"

namespace gdm {

using ShaderBlob = ID3DBlob;

struct DxShaderLoader
{
  DxShaderLoader(const char* fname, ID3D11Device* device, gfx::EShaderType type);
  virtual ~DxShaderLoader();

  auto GetBlob() -> ID3DBlob* { return shader_blob_; }

private:
  ID3DBlob* CreateShaderBlobFromCbo(const char* fname_cbo) const;
  ID3DBlob* CreateShaderBlobFromHlsl(const char* fname_hlsl, ID3D11Device* device, gfx::EShaderType type) const;
  const char* GetLatestFeatureLevel(D3D_FEATURE_LEVEL curr_feature_lvl, gfx::EShaderType type) const;

private:
  ID3DBlob* shader_blob_;

}; // struct DxShaderLoader

} // namespace gdm

#endif // AH_DX_SHADER_LOADER_H