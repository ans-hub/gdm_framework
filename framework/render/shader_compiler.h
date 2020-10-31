// *************************************************************
// File:    shader_compiler.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_SHADER_COMPILER_H
#define GM_SHADER_COMPILER_H

#include <string>
#include <vector>

#include <dxc/dxcapi.h>
#include <dxc/Support/dxcapi.use.h>

#include <render/defines.h>

namespace gdm {

struct ShaderBlob
{
  void* ptr;
  uint32_t size;
};

struct ShaderCompiler
{
  ShaderCompiler(const std::vector<LPCWSTR>& args);
  auto Compile(const char* fpath, gfx::EShaderType type) -> ShaderBlob;

private:
  dxc::DxcDllSupport dxc_support_;
  IDxcLibrary* dxc_library_;
  IDxcCompiler* dxc_compiler_;
  std::vector<LPCWSTR> args_;

}; // struct ShaderCompiler

} // namespace gdm

#include "shader_compiler.inl"

#endif // GM_SHADER_COMPILER_H
