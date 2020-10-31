// *************************************************************
// File:    model_loader.cc
// Descr:   represents abstract model loader
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "model_loader.h"

#include <memory>
#include <algorithm>
#include <utility>

#include "system/string_utils.h"

// --public

gdm::ModelLoader::ModelLoader(const char* model_fname, const char* material_path)
  : model_fname_{model_fname}
  , material_path_{material_path}
  , type_{}
  , loader_ply_{}
  , material_ply_{}
  , loader_obj_{}
{
  if (!material_path_.empty() && material_path_.back() != '/')
    material_path_.push_back('/');

  std::string extension = str::GetExtensionFromFpath(model_fname_.c_str());

  if (extension == "ply")
  {
    type_ = ELoaderType::PLY;
    loader_ply_ = std::make_unique<ply::Loader>(model_fname_.c_str());

    std::string mat_fname = loader_ply_->GetString("material").c_str();
    std::string mat_full_path = material_path_ + mat_fname;
    material_ply_ = Config(mat_full_path.c_str());
  }
  else if (extension == "obj")
  {
    type_ = ELoaderType::OBJ;
    loader_obj_ = std::make_unique<obj::Loader>(model_fname_, material_path_);
  }
}

const char* gdm::ModelLoader::GetFilePath() const
{
  return model_fname_.c_str();
}

int gdm::ModelLoader::GetMeshesCount() const
{
  switch (type_)
  {
    case ELoaderType::PLY : return 1;
    case ELoaderType::OBJ : return static_cast<int>(loader_obj_->LoadedMeshes.size());
  }
  return -1;
}

int gdm::ModelLoader::GetMaterialsCount() const
{
  switch (type_)
  {
    case ELoaderType::PLY : return 1;
    case ELoaderType::OBJ : return static_cast<int>(loader_obj_->LoadedMaterials.size());
  }
  return -1;
}

const char* gdm::ModelLoader::GetMaterialName(std::size_t mat_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY :
    {
      return material_ply_.Get<const std::string&>("name").c_str();
    }
    case ELoaderType::OBJ :
    {
      assert(mat_num < loader_obj_->LoadedMaterials.size());
      return loader_obj_->LoadedMaterials[mat_num].name.c_str();
    }
  }
  return {""};
}

const char* gdm::ModelLoader::GetMeshName(std::size_t mesh_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY : return {""};
    case ELoaderType::OBJ :
    {
      assert(mesh_num < loader_obj_->LoadedMeshes.size());
      return loader_obj_->LoadedMeshes[mesh_num].MeshName.c_str();
    }
  }
  return {""};
}

const char* gdm::ModelLoader::GetMeshMaterialName(std::size_t mesh_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY : return material_ply_.Get<const std::string&>("name").c_str();
    case ELoaderType::OBJ :
    {
      assert(mesh_num < loader_obj_->LoadedMeshes.size());
      return loader_obj_->LoadedMeshes[mesh_num].MeshMaterial.name.c_str();
    }
  }
  return {""};
}
