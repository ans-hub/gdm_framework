
// *************************************************************
// File:    mesh.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "mesh.h"
#include "gl_pointers.h"

#include "data/ply_loader.h"
#include "system/array_utils.h"

namespace gdm {

Mesh::Mesh(ply::Loader& ply)
  : faces_{}
  , coords_{}
  , normals_{}
  , tangents_{}
  , texuv_{}
  , vx_total_sz_{}
  , interleaving_vxs_buffer_ {}
  , interleaving_vxs_buffer_sz_{}
  , vxs_buffer_offsets_(VxElements::MAX)
  , tm_(1.f)
  , vao_{}
{
  // Load / create data

  faces_ = ply::helpers::LoadFaces<Vec3u>(ply);
  assert(!faces_.empty());

  coords_ = ply::helpers::LoadXYZ<Vec3f>(ply);
  arr_utils::normalize(&coords_[0].x, coords_.size() * 3, -1.f, 1.f);
  assert(!coords_.empty());

  normals_ = ply::helpers::LoadNormals<Vec3f>(ply);
  if (normals_.empty())
    normals_ = mesh_utils::ComputeVxsNormals(coords_, faces_);
  assert(normals_.size() == coords_.size());

  texuv_ = ply::helpers::LoadUV<Vec2f>(ply);
  if (texuv_.empty())
    texuv_.resize(coords_.size(), Vec2f{0.5f, 0.5f}); // todo: magic const
  assert(coords_.size() == texuv_.size());

  // todo: make separated structs just for tg - not mxs

  std::vector<Mat4f> vxs_tbns = mesh_utils::ComputeVxsTangentSpace(coords_, texuv_, normals_, faces_);  // todo if we havent 
  assert(vxs_tbns.size() == coords_.size());
  std::vector<Vec3f> tangents {};
  for (const auto& tbn : vxs_tbns)
    tangents_.push_back(vec3::Normalize(tbn.GetCol(0)));
  assert(tangents_.size() == coords_.size());

  // Make continuous array from elements above

  AddToVxsBuffer(coords_, VxElements::COORDS);
  AddToVxsBuffer(texuv_, VxElements::TEXUV);
  AddToVxsBuffer(normals_, VxElements::NORMALS);
  AddToVxsBuffer(tangents_, VxElements::TANGENTS);

  // Bind buffers

  assert(vx_total_sz_ > 0);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  GLuint vbo {0};
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, VxsBufferSize(), GetVxsBuffer(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VxSize(), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VxSize(), (GLvoid*)GetVxBufferOffset(VxElements::TEXUV));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VxSize(), (GLvoid*)GetVxBufferOffset(VxElements::NORMALS));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, VxSize(), (GLvoid*)GetVxBufferOffset(VxElements::TANGENTS));
  glEnableVertexAttribArray(3);

  GLuint ibo {0};
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, FacesSize(), GetFaces(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Mesh::Draw() const
{
  glBindVertexArray(vao_);
  glDrawElements(GL_TRIANGLES, FacesCount() * 3, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

} // namespace gdm
