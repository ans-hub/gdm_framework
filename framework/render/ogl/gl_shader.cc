// *************************************************************
// File:    shader.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "shader.h"

#include <fstream>
#include <sstream>
#include <cassert>

namespace gdm {

Shader::Shader(const char* vert_path, const char* frag_path,
               const char* geom_path)
  : prog_id_{}
{
  assert(vert_path != nullptr && frag_path != nullptr);

  prog_id_ = glCreateProgram();

  GLuint vert_sh = Compile(GL_VERTEX_SHADER, vert_path);
  GLuint frag_sh = Compile(GL_FRAGMENT_SHADER, frag_path);
  GLuint geom_sh = Compile(GL_GEOMETRY_SHADER, geom_path);

  Attach(vert_sh, frag_sh, geom_sh);
  Link();
  Delete(vert_sh, frag_sh, geom_sh);
}

GLuint Shader::Compile(GLenum sh_type, const char* sh_path)
{
  if (sh_path == nullptr)
    return 0;

  GLuint sh_id = glCreateShader(sh_type);
  std::string sh_text = shader_utils::ReadShaderContents(sh_path).c_str();
  const GLchar* sh_code = sh_text.c_str();
  glShaderSource(sh_id, 1, &sh_code, nullptr);
  glCompileShader(sh_id);
  
  int compile_status {};
  glGetShaderiv(sh_id, GL_COMPILE_STATUS, &compile_status);
  if (compile_status != GL_TRUE)
  {
    std::string msg = shader_utils::GetShaderInfoLog(sh_id, sh_path);
    throw ShaderExcept(msg.c_str());
  }
  return sh_id;
}

void Shader::Attach(GLuint vert_sh, GLuint frag_sh, GLuint geom_sh)
{
  if (vert_sh != 0)
    glAttachShader(prog_id_, vert_sh);
  if (frag_sh != 0)
    glAttachShader(prog_id_, frag_sh);
  if (geom_sh != 0)
    glAttachShader(prog_id_, geom_sh);
}

void Shader::Link()
{
  glLinkProgram(prog_id_);

  int linking_status {};
  glGetProgramiv(prog_id_, GL_LINK_STATUS, &linking_status);
  if (linking_status != GL_TRUE)
  {
    std::string msg = "Shader: linking failed";
    throw ShaderExcept(msg.c_str());
  }
}

void Shader::Delete(GLuint vert_sh, GLuint frag_sh, GLuint geom_sh)
{
  if (vert_sh != 0)
    glDeleteShader(vert_sh);
  if (frag_sh != 0)
    glDeleteShader(frag_sh);
  if (geom_sh != 0)
    glDeleteShader(geom_sh);
}

void Shader::Use() const
{ 
  glUseProgram(prog_id_);
}

Shader::~Shader()
{
  Release();
}

void Shader::Release()
{
  prog_id_ = 0;
}

Shader::Shader(Shader&& shader)
  : prog_id_{shader.prog_id_}
{
  shader.prog_id_ = 0;
}

Shader& Shader::operator=(Shader&& shader)
{
  if(this != &shader)
  {
    Release();
    std::swap(prog_id_, shader.prog_id_);
  }
  return *this;
}

std::string shader_utils::ReadShaderContents(const char* fpath)
{
  std::ifstream ifs {fpath};
  if (!ifs)
  {
    std::string msg = "Shader: file not found: " + std::string(fpath);
    throw ShaderExcept(msg.c_str());
  }
  std::string out {};
  std::string line {};
  while(std::getline(ifs, line))
    out.append(line + '\n');
  return out;
}

bool shader_utils::IsShaderCompiled(GLuint shader)
{
  int status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  return status == GL_TRUE;
}

bool shader_utils::IsProgramLinked(GLuint program)
{
  int status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  return status == GL_TRUE;
}

std::string shader_utils::GetShaderInfoLog(GLuint shader_id, const char* shader_name)
{
  char info[256];
  glGetShaderInfoLog(shader_id, 256, nullptr, info);
  std::ostringstream oss {};
  oss << "Shader " << shader_name << " compilation error\n" << info << '\n';
  return oss.str();
}
} // namespace gdm