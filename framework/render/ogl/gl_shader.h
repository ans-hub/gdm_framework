// *************************************************************
// File:    gl_shader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GL_SHADER_H
#define AH_GL_SHADER_H

#include <stdexcept>
#include <string>

#include <GL/glx.h>

#include "gl_pointers.h"

namespace gdm {

struct Shader
{
  Shader(const char* vert_path, const char* frag_path, const char* geom_path = nullptr);
  ~Shader();
  Shader(const Shader& tex) =delete;
  Shader& operator=(const Shader& tex) =delete;
  Shader(Shader&& tex);
  Shader& operator=(Shader&& tex);

public:
  void Use() const;

public:
  template<class T>
  void SetVal(const char* name, T val) const;
  template<size_t N>
  void SetVec(const char* name, const float* val) const;
  template<size_t N>
  void SetMat(const char* name, const float* val) const;

private:
  GLuint Compile(GLenum sh_type, const char* sh_path);
  void Attach(GLuint vert_sh, GLuint frag_sh, GLuint geom_sh);
  void Link();
  void Delete(GLuint vert_sh, GLuint frag_sh, GLuint geom_sh);
  void Release();

private:
  GLuint prog_id_;

}; // struct Shader

struct ShaderExcept : std::runtime_error
{
  ShaderExcept(const char* msg) : std::runtime_error(msg) { }

}; // struct Except

namespace shader_utils {

  std::string ReadShaderContents(const char* fpath);
  std::string GetShaderInfoLog(GLuint shader_id, const char* shader_name);
  bool IsShaderCompiled(GLuint shader);
  bool IsProgramLinked(GLuint program);

} // namespace shader_utils

template<>
inline void Shader::SetVal<bool>(const char* name, bool val) const
{
  glUniform1i(glGetUniformLocation(prog_id_, name), (int)val); 
}

template<>
inline void Shader::SetVal<int>(const char* name, int val) const
{
  glUniform1i(glGetUniformLocation(prog_id_, name), (int)val); 
}

template<>
inline void Shader::SetVal<float>(const char* name, float val) const
{
  glUniform1f(glGetUniformLocation(prog_id_, name), val); 
}

template<>
inline void Shader::SetVec<2>(const char* name, const float* vec) const
{
  glUniform2fv(glGetUniformLocation(prog_id_, name), 1, vec);
}

template<>
inline void Shader::SetVec<3>(const char* name, const float* vec) const
{
  glUniform3fv(glGetUniformLocation(prog_id_, name), 1, vec);
}

template<>
inline void Shader::SetVec<4>(const char* name, const float* vec) const
{
  glUniform4fv(glGetUniformLocation(prog_id_, name), 1, vec);
}

template<>
inline void Shader::SetMat<3>(const char* name, const float* mat) const
{
  glUniformMatrix3fv(glGetUniformLocation(prog_id_, name), 1, GL_FALSE, mat);
}

template<>
inline void Shader::SetMat<4>(const char* name, const float* mat) const
{
  glUniformMatrix4fv(glGetUniformLocation(prog_id_, name), 1, GL_FALSE, mat);
}

} // namespace gdm

#endif // AH_GL_SHADER_H