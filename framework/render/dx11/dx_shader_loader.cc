// *************************************************************
// File:    dx_shader_loader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#define DEBUG_LEVEL 1

#include "dx_shader_loader.h"

// --public

gdm::DxShaderLoader::DxShaderLoader(const char* fname, ID3D11Device* device, gfx::EShaderType type)
  : shader_blob_{nullptr}
{
  shader_blob_ = CreateShaderBlobFromHlsl(fname, device, type);
  if (!shader_blob_)
    shader_blob_ = CreateShaderBlobFromCbo(fname);
  if (!shader_blob_)
  {
    ASSERTF(false, "Couldn't load shader either from hlsl or from bin");
  }
}

gdm::DxShaderLoader::~DxShaderLoader()
{
  d3d_utils::Release(shader_blob_);
}

// --private

ID3DBlob* gdm::DxShaderLoader::CreateShaderBlobFromCbo(const char* fname_cbo) const
{
  ID3DBlob* shader_blob = nullptr;
  std::wstring fname_cbo_w = str::Ansi2Utf(fname_cbo);
  HRESULT hr = D3DReadFileToBlob(fname_cbo_w.c_str(), &shader_blob);
  ASSERTF(hr == 0, "Failed to read cso file to blob for shader - %d", hr);
  return shader_blob;
}

ID3DBlob* gdm::DxShaderLoader::CreateShaderBlobFromHlsl(const char* fname_hlsl, ID3D11Device* device, gfx::EShaderType type) const
{
  const char* entry_point = "main";
  const char* profile = GetLatestFeatureLevel(device->GetFeatureLevel(), type);
  ID3DBlob* shader_blob;
  ID3DBlob* error_blob;

  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif

  bool file_exists = std::filesystem::exists(fname_hlsl);
  ASSERTF(file_exists, "File not found: %s", fname_hlsl);

  std::wstring fname_hlsl_w = str::Ansi2Utf(fname_hlsl);
  HRESULT hr = D3DCompileFromFile(fname_hlsl_w.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                  entry_point, profile, flags, 0, &shader_blob, &error_blob);
  ASSERTF(hr == 0, "Couldn't create shader blob %s", (char*)error_blob->GetBufferPointer());

  d3d_utils::Release(error_blob);
  return shader_blob;
}

const char* gdm::DxShaderLoader::GetLatestFeatureLevel(D3D_FEATURE_LEVEL curr_feature_lvl, gfx::EShaderType type) const
{
  if (type == gfx::EShaderType::VX)
  {
    switch(curr_feature_lvl)
    {
      case D3D_FEATURE_LEVEL_11_1: return "vs_5_0";
      case D3D_FEATURE_LEVEL_11_0: return "vs_5_0";
      case D3D_FEATURE_LEVEL_10_1: return "vs_4_1";
      case D3D_FEATURE_LEVEL_10_0: return "vs_4_0";
      case D3D_FEATURE_LEVEL_9_3: return "vs_4_0_level_9_3";
      case D3D_FEATURE_LEVEL_9_2: return "vs_4_0_level_9_1";
      case D3D_FEATURE_LEVEL_9_1: return "vs_4_0_level_9_1";
      default : return "";
    }
  }
  if (type == gfx::EShaderType::PX)
  {
    switch(curr_feature_lvl)
    {
      case D3D_FEATURE_LEVEL_11_1: return "ps_5_0";
      case D3D_FEATURE_LEVEL_11_0: return "ps_5_0";
      case D3D_FEATURE_LEVEL_10_1: return "ps_4_1";
      case D3D_FEATURE_LEVEL_10_0: return "ps_4_0";
      case D3D_FEATURE_LEVEL_9_3: return "ps_4_0_level_9_3";
      case D3D_FEATURE_LEVEL_9_2: return "ps_4_0_level_9_1";
      case D3D_FEATURE_LEVEL_9_1: return "ps_4_0_level_9_1";
      default : return "";
    }
  }
  return "";
}
