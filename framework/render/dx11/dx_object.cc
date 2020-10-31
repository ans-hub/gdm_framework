// *************************************************************
// File:    dx_object.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_object.h"
#include "dx_renderer.h" 

#include "system/string_utils.h"

// --public

namespace gdm {

DxObject::DxObject(DxRenderer& d3d, const char* model_fpath)
  : tm_(1.f)
  , meshes_{}
  , materials_{}
  , name_{}
{
  ASSERT(model_fpath);
  std::string mat_fpath = str::GetPathFromFullPath(model_fpath);
  ModelLoader model(model_fpath, mat_fpath.c_str());

  for (std::size_t i = 0; i < model.GetMaterialsCount(); ++i)
  {
    DxMaterial material(model, d3d.GetDevice(), i);
    materials_.push_back(std::move(material));
  }
  for (std::size_t i = 0; i < model.GetMeshesCount(); ++i)
  {
    DxMesh mesh(model, d3d.GetDevice(), i);
    DxMaterial* material = FindMaterial(model.GetMeshMaterialName(i));
    ASSERTF(material, "Couldn't load material %s for mesh num %i", model.GetMeshMaterialName(i), (int)i);
    mesh.SetMaterial(material, d3d.GetDevice());
    meshes_.push_back(std::move(mesh));
  }
}

DxMaterial* DxObject::FindMaterial(const std::string& find)
{
  for (DxMaterial& material : materials_)
  {
    if (material.GetName() == find)
      return &material;
  }
  return nullptr;
}

} // namespace gdm
