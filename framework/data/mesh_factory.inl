// *************************************************************
// File:    mesh_factory.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "mesh_factory.h"

// --private

template<class T>
inline void gdm::MeshFactory::AddToVxsBuffer(AbstractMesh* mesh, const std::vector<T>& v)
{
  assert(!v.empty());
  std::size_t elems_cnt = sizeof(T) / sizeof(T::x);

  if (mesh->interleaving_vxs_buffer_ .empty())
    mesh->interleaving_vxs_buffer_  = arr_utils::ArrayToVector(&v[0].x, v.size(), elems_cnt);
  else
    mesh->interleaving_vxs_buffer_  = arr_utils::Expand(&mesh->interleaving_vxs_buffer_[0], mesh->interleaving_vxs_buffer_sz_,
                                      mesh->interleaving_vxs_buffer_ .size() / mesh->interleaving_vxs_buffer_sz_,
                                      &v[0].x, elems_cnt, v.size());
  mesh->vx_total_sz_ += sizeof(T);
  mesh->interleaving_vxs_buffer_sz_ += elems_cnt;

  assert(mesh->interleaving_vxs_buffer_ .size() == v.size() * mesh->interleaving_vxs_buffer_sz_);
}