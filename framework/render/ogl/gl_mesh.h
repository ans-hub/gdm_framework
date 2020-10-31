
// *************************************************************
// File:    gl_mesh.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// https://www.khronos.org/opengl/wiki/Vertex_Specification_Best_Practices#Formatting_VBO_Data

#ifndef AH_GL_MESH_H
#define AH_GL_MESH_H

#include <vector>
#include <GL/gl.h>

#include <math/matrix.h>
#include <math/vector2.h>
#include <math/vector3.h>
#include <system/array_utils.h>
#include <render/mesh_utils.h>

namespace ply {

  struct Loader;
}

namespace gdm {

class Mesh
{
  enum VxElements
  {
    COORDS = 0,
    TEXUV,
    NORMALS,
    TANGENTS,
    MAX

  }; // enum VxElements

  std::vector<Vec3u> faces_;
  std::vector<Vec3f> coords_;
  std::vector<Vec3f> normals_;
  std::vector<Vec3f> tangents_;
  std::vector<Vec2f> texuv_;

  float vx_total_sz_;
  std::vector<float> interleaving_vxs_buffer_ ;
  std::size_t interleaving_vxs_buffer_sz_;
  std::vector<unsigned int> vxs_buffer_offsets_;

  Mat4f tm_;
  GLuint vao_;

  template<class T>
    void AddToVxsBuffer(const std::vector<T>& v, VxElements elem_type);

public:
  Mesh(ply::Loader& ply);

  void Draw() const;
  const unsigned int* GetFaces() const { return &faces_[0].x; }
  const float* GetVxsBuffer() const { return &interleaving_vxs_buffer_ [0]; }
  const std::vector<Vec3f>& GetVxsTangents() const { return tangents_; }
  const std::vector<Vec3f>& GetVxsCoords() const { return coords_; }
  const std::vector<Vec3f>& GetVxsNormals() const { return normals_; }

  Mat4f& GetTm() { return tm_; } 
  void SetTm(const Mat4f& tm) { tm_ = tm; }
  void SetPos(const Vec3f& pos) { tm_.SetCol(3, pos); }
  Vec3f GetPos() const { return tm_.GetCol(3); }

  auto VxSize() const { return vx_total_sz_; }
  auto VxsCount() const { return coords_.size(); }
  auto VxsSize() const { return coords_.size() * VxSize(); }
  auto FacesCount() const { return faces_.size(); }
  auto FacesSize() const { return faces_.size() * 3 * sizeof(unsigned int); }
  auto VxsBufferCount() const { return interleaving_vxs_buffer_ .size(); }
  auto VxsBufferSize() const { return interleaving_vxs_buffer_ .size() * sizeof(float); }
  auto NormalsCount() const { return normals_.size(); }
  auto CoordsCount() const { return coords_.size(); }
  std::size_t GetVxBufferOffset(VxElements type) const { return vxs_buffer_offsets_[type]; }

}; // struct Model

template<class T>
inline void Mesh::AddToVxsBuffer(const std::vector<T>& v, VxElements elem_type)
{
  assert(!v.empty());
  std::size_t elems_cnt = sizeof(T) / sizeof(T::x);

  if (interleaving_vxs_buffer_ .empty())
    interleaving_vxs_buffer_  = arr_utils::ArrayToVector(&v[0].x, v.size(), elems_cnt);
  else
    interleaving_vxs_buffer_  = arr_utils::expand(&interleaving_vxs_buffer_ [0], interleaving_vxs_buffer_sz_, interleaving_vxs_buffer_ .size() / interleaving_vxs_buffer_sz_,
                                    &v[0].x, elems_cnt, v.size());
  vx_total_sz_ += sizeof(T);
  interleaving_vxs_buffer_sz_ += elems_cnt;
  vxs_buffer_offsets_[elem_type] = vx_total_sz_ - sizeof(T);

  assert(interleaving_vxs_buffer_ .size() == v.size() * interleaving_vxs_buffer_sz_);
}

} // namespace gdm

#endif // AH_GL_MESH_H
