
// *************************************************************
// File:    debug_draw.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <cassert>

#include "gl_debug_draw.h"

namespace gdm {

void DebugDraw::Reinit()
{
  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  int vbo_sz_bytes = k_max_buffer_size_ * sizeof(Vec4f);
  glBufferData(GL_ARRAY_BUFFER, vbo_sz_bytes, &lines_[0], GL_STREAM_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
  
  glBindVertexArray(0);
}

void DebugDraw::AddLine(const Vec3f& v1, const Vec3f& v2, unsigned int color)
{
  assert(lines_.size() < k_max_buffer_size_);
  lines_.push_back(Vec4f(v1, static_cast<float>(color)));
  lines_.push_back(Vec4f(v2, static_cast<float>(color)));
}

void DebugDraw::AddDirs(const std::vector<Vec3f>& vxs_coords, const std::vector<Vec3f>& dirs,
                        float len, unsigned int color)
{
  for (std::size_t i = 0; i < dirs.size(); ++i)
    AddLine(vxs_coords[i], vxs_coords[i] + dirs[i] * len, color);
}

void DebugDraw::Draw()
{
  Reinit();
  glBindVertexArray(vao_);
  glDrawArrays(GL_LINES, 0, lines_.size());
  glBindVertexArray(0);
  lines_.clear();
}

}  // namespace gdm
