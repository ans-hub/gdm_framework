
// *************************************************************
// File:    gl_debug_draw.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DEBUG_DRAW_H
#define AH_DEBUG_DRAW_H

#include <vector>
#include <functional>
#include <GL/gl.h>

#include "gl_pointers.h"

#include <math/vector3.h>
#include <math/vector4.h>

namespace gdm {

class DebugDraw
{
  GLuint vao_ = 0;
  std::vector<Vec4f> lines_ = {};
  const float k_max_buffer_size_ = 2048 * 4;

  void Reinit();

public:
  void AddLine(const Vec3f& v1, const Vec3f& v2, unsigned int color);
  void AddDirs(const std::vector<Vec3f>& vxs_coords, const std::vector<Vec3f>& dirs,
               float len, unsigned int color);
  void Draw();

}; // struct DebugDraw

} // namespace gdm

#endif // AH_DEBUG_DRAW_H
