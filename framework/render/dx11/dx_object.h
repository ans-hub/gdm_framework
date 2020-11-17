// *************************************************************
// File:    dx_object.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_OBJECT_H
#define AH_DX_OBJECT_H

#include <d3d11.h>
#include <vector>
#include <string>

#include <data/model_loader.h>
#include <math/matrix.h>
#include <math/vector3.h>
#include <system/assert_utils.h>

#include "dx_mesh.h"
#include "dx_material.h"

namespace gdm {

struct DxRenderer;

struct DxObject
{
  DxObject() =default;
  DxObject(DxRenderer& d3d, const char* model_fpath);

  Mat4f& GetTm() { return tm_; } 
  Mat4f GetItm() { return matrix::InverseTransform(matrix::SwapForwardRight(tm_)); }
  Vec3f GetPos() const { return tm_.GetCol(3); }
  void SetTm(const Mat4f& tm) { tm_ = tm; }
  void SetPos(const Vec3f& pos) { tm_.SetCol(3, pos); }
  auto GetMeshes() -> std::vector<DxMesh>& { return meshes_; }
  auto GetMeshes() const -> const std::vector<DxMesh>& { return meshes_; }
  void SetName(unsigned int name) { name_ = name; }
  auto GetName() const -> unsigned int { return name_; }

private:
  DxMaterial* FindMaterial(const std::string& find);

private:
  Mat4f tm_;
  std::vector<DxMesh> meshes_;
  std::vector<DxMaterial> materials_;
  unsigned int name_;

}; // struct DxObject

} // namespace gdm

#endif // AH_DX_OBJECT_H