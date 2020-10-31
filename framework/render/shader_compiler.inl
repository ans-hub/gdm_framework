// *************************************************************
// File:    shader_compiler.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "shader_compiler.h"

#include <fstream>

#include "system/assert_utils.h"
#include "system/string_utils.h"

// --private

namespace gdm::_private{

  constexpr const wchar_t* v_entry_point { L"main" }; 
  constexpr const wchar_t* v_profiles[gfx::EShaderType::Size]
    { L"vs_6_1", L"ps_6_1", L"cs_6_1", L"gs_6_1" };
}

// --public

inline gdm::ShaderCompiler::ShaderCompiler(const std::vector<LPCWSTR>& args)
  : args_{args}
{
  HRESULT res = dxc_support_.Initialize();
  ASSERTF(res == S_OK, "ShaderCompiler failed %s", error::LastError(res).c_str());

  res = dxc_support_.CreateInstance<IDxcLibrary>(CLSID_DxcLibrary, &dxc_library_);
  ASSERTF(res == S_OK, "ShaderCompiler failed %s", error::LastError(res).c_str());

  res = dxc_support_.CreateInstance(CLSID_DxcCompiler, &dxc_compiler_);
  ASSERTF(res == S_OK, "ShaderCompiler failed %s", error::LastError(res).c_str());
}

inline auto gdm::ShaderCompiler::Compile(const char* fpath, gfx::EShaderType type) -> ShaderBlob
{
  uint codepage = CP_UTF8;
  std::wstring fpath_w = str::Ansi2Utf(fpath, codepage);
  IDxcBlobEncoding* blob;

  HRESULT res = dxc_library_->CreateBlobFromFile(fpath_w.c_str(), &codepage, &blob);
  ASSERTF(res == S_OK, "ShaderCompiler->CreateBlobFromFile() failed %s", error::LastError(res).c_str());

  LPCWSTR shader_name = fpath_w.c_str();
  IDxcOperationResult* result;

  res = dxc_compiler_->Compile(blob, shader_name, _private::v_entry_point, _private::v_profiles[type],
                               args_.data(), static_cast<UINT32>(args_.size()), nullptr, 0, nullptr, &result);

  IDxcBlob* result_blob = nullptr;
  IDxcBlobEncoding* err_blob = nullptr;

  if (res == S_OK)
  {
    res = result->GetResult(&result_blob);
    ASSERTF(res == S_OK, "ShaderCompiler->GetResult() failed %s", error::LastError(res).c_str());
    res = result->GetErrorBuffer(&err_blob);
    ASSERTF(res == S_OK, "ShaderCompiler->GetErrorBuffer() failed %s", error::LastError(res).c_str());
  }

  ASSERTF(result_blob, "Shader compile error: %s", static_cast<char*>(err_blob->GetBufferPointer()));
  ASSERTF(result_blob->GetBufferPointer(), "Shader compile error: %s", static_cast<char*>(err_blob->GetBufferPointer()));

  return {result_blob->GetBufferPointer(), static_cast<uint32_t>(result_blob->GetBufferSize())};
}
