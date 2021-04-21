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
  using VertexType = float;
  using IndexType = unsigned int;
  using FaceType = Vec3u;

  constexpr static int v_vxs_in_face = 3;

public:
  template<class T>
  void SetVertexBuffer(T* buffer) { vertex_buffer_ = static_cast<void*>(buffer); }
  template<class T>
  void SetIndexBuffer(T* buffer) { index_buffer_ = static_cast<void*>(buffer); }
  template<class T>
  auto GetVertexBuffer() const -> T* { return static_cast<T*>(vertex_buffer_); }
  template<class T>
  auto GetIndexBuffer() const -> T* { return static_cast<T*>(index_buffer_); }

// private:
  std::vector<Vec3u> faces_;
  std::vector<Vec3f> coords_;
  std::vector<Vec3f> normals_;
  std::vector<Vec3f> tangents_;
  std::vector<Vec2f> texuv_;
  std::size_t vx_total_sz_;
  std::vector<float> interleaving_vxs_buffer_ ;
  std::size_t interleaving_vxs_buffer_sz_;
  MaterialHandle material_;
  void* vertex_buffer_ = nullptr;
  void* index_buffer_ = nullptr;

public:
  static const bool normalize_poses_ = false;

}; // struct AbstractMesh

struct MeshFactory : public DataFactory<AbstractMesh*>
{
  static auto Create(const char* mesh_name, int mesh_num, const ModelLoader& loader) -> MeshHandle;
  static void Release(MeshHandle handle);
  static bool ImplementationLoaded(MeshHandle handle);

private:
  template<class T>
  static void AddToVxsBuffer(AbstractMesh* mesh, const std::vector<T>& v);

}; // struct MeshFactory

} // namespace gdm

#include "mesh_factory.inl"

#endif // AH_GDM_MESH_FACT_H
