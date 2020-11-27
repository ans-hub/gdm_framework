// *************************************************************
// File:    shader.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "shader.h"

#include "memory/defines.h"

// --private

inline auto gdm::Shader::CreateCompiler() -> gdm::ShaderCompiler*
{
  std::vector<LPCWSTR> args {
    L"-fspv-target-env=vulkan1.2",
    L"-spirv",
    L"-fvk-use-dx-layout",
    L"-auto-binding-space 0",
    L"-fvk-use-dx-position-w"
  };
  if (gfx::v_DebugBuild)
  {
    args.push_back(L"-Od");
    args.push_back(L"-Zi");
    args.push_back(L"-Qembed_debug");
  }
  else
  {
    args.push_back(L"-O3");
  }
  return GMNew ShaderCompiler(args);
}

inline gdm::ShaderCompiler* gdm::Shader::compiler_ = gdm::Shader::CreateCompiler();

// --public

inline gdm::Shader::Shader(const char* fpath, gfx::EShaderType type)
  : shader_blob_{ compiler_->Compile(fpath, type) }
  , type_{type}
{ }

inline gdm::Shader::~Shader()
{
  // todo: compiler_->Release(shader_blob_);
}
