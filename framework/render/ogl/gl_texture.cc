// *************************************************************
// File:    gl_texture.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "texture.h"

#include <algorithm>

#include "gl_pointers.h"

namespace gdm {

Texture2d::Texture2d(int w, int h, int depth, const unsigned char* data, bool gamma_corrected)
  : w_{w}
  , h_{h}
  , depth_{depth}
  , tex_id_{0}
{
  glGenTextures(1, &tex_id_);
  glBindTexture(GL_TEXTURE_2D, tex_id_);
  bool alpha = (depth != 24);
  GLenum internal_format = gamma_corrected ? (alpha ? GL_SRGB_ALPHA : GL_SRGB) : (alpha ? GL_RGBA : GL_RGB);
  GLenum format = alpha ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w_, h_, 0, format, GL_UNSIGNED_BYTE, data);

  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

Texture2d::~Texture2d()
{
  Release();
}

void Texture2d::Release()
{
  glDeleteTextures(1, &tex_id_);
  tex_id_ = 0;
}

Texture2d::Texture2d(Texture2d&& tex)
  : tex_id_{tex.tex_id_}
{
  tex.tex_id_ = 0;
}

Texture2d& Texture2d::operator=(Texture2d&& tex)
{
  if(this != &tex)
  {
    Release();
    std::swap(tex_id_, tex.tex_id_);
  }
  return *this;
}

void Texture2d::Activate(std::size_t tex_num) const
{
  glActiveTexture(GL_TEXTURE0 + tex_num);
  glBindTexture(GL_TEXTURE_2D, tex_id_);
}

} // namespace gdm