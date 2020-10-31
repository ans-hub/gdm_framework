// *************************************************************
// File:    gl_texture.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GL_TEXTURE_H
#define AH_GL_TEXTURE_H

#include <fstream>
#include <GL/glx.h>

#include "gl_pointers.h"

namespace gdm {

struct Texture2d
{
  template<class T> Texture2d(const T& img, bool gamma_corrected = true);
  Texture2d(int w, int h, int depth, const unsigned char* data, bool gamma_corrected = true);
  ~Texture2d();

  Texture2d(const Texture2d& tex) =delete;
  Texture2d& operator=(const Texture2d& tex) =delete;
  Texture2d(Texture2d&& tex);
  Texture2d& operator=(Texture2d&& tex);

  void Activate(std::size_t tex_num) const;

private:
  void Release();

private:
  int w_;
  int h_;
  int depth_;
  GLuint tex_id_;

}; // struct Texture2d

template<class Img>
inline Texture2d::Texture2d(const Img& img, bool gamma_corrected)
  : Texture2d(img.GetWidth(), img.GetHeight(), img.GetDepth(), img.GetRaw(), gamma_corrected)
{ }

} // namespace gdm

#endif // AH_GL_TEXTURE_H