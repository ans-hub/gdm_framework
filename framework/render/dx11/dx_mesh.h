// *************************************************************
// File:    dx_mesh.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_MESH_H
#define AH_DX_MESH_H

#include <d3d11.h>
#include <vector>
#include <string>

#include <data/helpers.h>
#include <data/model_loader.h>
#include <data/abstract_image.h>
#include <system/array_utils.h>
#include <system/assert_utils.h>
#include <math/vector3.h>
#include <system/mesh_utils.h>

#include "dx_material.h"

namespace gdm {

struct DxRenderer;

struct DxMesh
{
  DxMesh(ModelLoader& model, ID3D11Device* device, std::size_t mesh_num = 0);
  ~DxMesh();

  DxMesh(const DxMesh& mesh) =delete;
  DxMesh& operator=(const DxMesh& mesh) =delete;
  DxMesh(DxMesh&& mesh);
  DxMesh& operator=(DxMesh&& mesh);

  void SetMaterial(DxMaterial* material, ID3D11Device* device/*, std::size_t mesh_num*/);
  ID3D11Buffer* GetVertexBuffer() { return vertex_buffer_; }
  ID3D11Buffer* GetIndexBuffer() { return index_buffer_; }
  ID3D11ShaderResourceView* GetDiffuseMap() { return material_->diffuse_map_.GetShaderResourceView(); }
  ID3D11ShaderResourceView* GetNormalMap() { return material_->normal_map_.GetShaderResourceView(); }
  ID3D11ShaderResourceView* GetSpecularMap() { return material_->specular_map_.GetShaderResourceView(); }
  std::size_t GetIndiciesCount() const { return faces_.size(); }
  std::size_t GetVerticesSize() const { return coords_.size(); }
  void* GetMaterialProps() { return (void*)&(material_->props_); }

private:
  std::vector<Vec3u> faces_;
  std::vector<Vec3f> coords_;
  std::vector<Vec3f> normals_;
  std::vector<Vec3f> tangents_;
  std::vector<Vec2f> texuv_;
  DxMaterial* material_;

  std::size_t vx_total_sz_;
  std::vector<float> interleaving_vxs_buffer_ ;
  std::size_t interleaving_vxs_buffer_sz_;
  bool normalize_poses_;

  ID3D11Buffer* vertex_buffer_;
  ID3D11Buffer* index_buffer_;

  void LoadData(ModelLoader& model, std::size_t mesh_num);
  void MergeData();
  void CreateVertexBuffer(ID3D11Device* device);
  void CreateIndexBuffer(ID3D11Device* device);

  template<class T>
    void AddToVxsBuffer(const std::vector<T>& v);

}; // struct DxMesh

template<class T>
inline void DxMesh::AddToVxsBuffer(const std::vector<T>& v)
{
  assert(!v.empty());
  std::size_t elems_cnt = sizeof(T) / sizeof(T::x);

  if (interleaving_vxs_buffer_ .empty())
    interleaving_vxs_buffer_  = arr_utils::ArrayToVector(&v[0].x, v.size(), elems_cnt);
  else
    interleaving_vxs_buffer_  = arr_utils::Expand(&interleaving_vxs_buffer_ [0], interleaving_vxs_buffer_sz_, interleaving_vxs_buffer_ .size() / interleaving_vxs_buffer_sz_,
                                    &v[0].x, elems_cnt, v.size());
  vx_total_sz_ += sizeof(T);
  interleaving_vxs_buffer_sz_ += elems_cnt;

  assert(interleaving_vxs_buffer_ .size() == v.size() * interleaving_vxs_buffer_sz_);
}

} // namespace gdm

#endif // AH_DX_MESH_H