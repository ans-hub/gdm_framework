// *************************************************************
// File:    mesh_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MESH_FACT_H
#define AH_GDM_MESH_FACT_H

#include <vector>

#include "memory/defines.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "data_factory.h"
#include "material_factory.h"

namespace gdm {

using MeshHandle = Handle;

struct AbstractMesh
{
  std::vector<Vec3u> faces_;
  std::vector<Vec3f> coords_;
  std::vector<Vec3f> normals_;
  std::vector<Vec3f> tangents_;
  std::vector<Vec2f> texuv_;
  std::size_t vx_total_sz_;
  std::vector<float> interleaving_vxs_buffer_ ;
  std::size_t interleaving_vxs_buffer_sz_;
  MaterialHandle material_;

public:
  static const bool normalize_poses_ = false;

public:
  void* impl_;

}; // struct AbstractMesh

struct MeshFactory : public DataFactory<AbstractMesh*>
{
  static auto Create(const char* mesh_name, int mesh_num, const ModelLoader& loader) -> MeshHandle;
  static void Release(MeshHandle handle);

private:
  template<class T>
  static void AddToVxsBuffer(AbstractMesh* mesh, const std::vector<T>& v);

}; // struct MeshFactory

} // namespace gdm

#include "mesh_factory.inl"

#endif // AH_GDM_MESH_FACT_H
