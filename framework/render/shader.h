// *************************************************************
// File:    shader.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_SHADER_H
#define GM_SHADER_H

#include <render/shader_compiler.h>

namespace gdm {

struct Shader
{
  Shader(const char* fpath, gfx::EShaderType type); // possible pass compiler here
  ~Shader();
  auto GetBlob() const -> const ShaderBlob& { return shader_blob_; }
  auto GetType() const -> gfx::EShaderType { return type_; }
private:
  static auto CreateCompiler() -> ShaderCompiler*;

private:
  static ShaderCompiler* compiler_; // possible: api::ShaderCompiler* compiler; 
  ShaderBlob shader_blob_;
  gfx::EShaderType type_;

}; // struct Shader

} // namespace gdm

#include "shader.inl"

#endif // GM_SHADER_H
