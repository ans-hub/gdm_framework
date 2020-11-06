// *************************************************************
// File:    model_loader.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <array>
#include <string>

#include "model_loader.h"
#include "ply_loader.h"
#include "obj_loader.h"

// --private

template<>
inline gdm::Vec4f gdm::ModelLoader::GetMaterial(const char* resource_name, std::size_t mat_num) const
{
  std::string res {resource_name};

  switch(type_)
  {
    case ELoaderType::PLY :
      return material_ply_.Has<Vec4f>(res) ? material_ply_.Get<Vec4f>(res) : Vec4f(0.f);
    case ELoaderType::OBJ :
    {
      if (loader_obj_->GetMaterials().empty())
        return {};

      assert(mat_num < loader_obj_->GetMaterials().size());
      const obj::material_t& mat = loader_obj_->GetMaterials()[mat_num];
      if (res == "emissive")
        return Vec4f{mat.emission[0], mat.emission[1], mat.emission[2], 0.f};  // needs to be zero for shader notify
      if (res == "ambient")
        return Vec4f{mat.ambient[0], mat.ambient[1], mat.ambient[2], 1.f};
      if (res == "diffuse")
        return Vec4f{mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.f};
      if (res == "specular")
        return Vec4f{mat.specular[0], mat.specular[1], mat.specular[2], 1.f};
    }
  }
  return {};
}

template<>
inline std::string gdm::ModelLoader::GetMaterial(const char* resource_name, std::size_t mat_num) const
{
  switch(type_)
  {
    case ELoaderType::PLY :
      return material_ply_.Get<std::string>(resource_name);
    case ELoaderType::OBJ :
    {
      if (loader_obj_->GetMaterials().empty())
        return {};

      assert(mat_num < loader_obj_->GetMaterials().size());
      const obj::material_t& material = loader_obj_->GetMaterials()[mat_num];

      if (strcmp(resource_name, "diffuse_map") == 0)
        return material.diffuse_texname.c_str();
      if (strcmp(resource_name, "specular_map") == 0)
        return material.specular_texname.c_str();
      if (strcmp(resource_name, "normal_map") == 0)
        return material.normal_texname.c_str();
      return {};
    }
  }
  return {};
}

template<>
inline float gdm::ModelLoader::GetMaterial(const char* resource_name, std::size_t mat_num) const
{
  std::string res {resource_name};

  switch(type_)
  {
    case ELoaderType::PLY :
      return material_ply_.Has<float>(res) ? material_ply_.Get<float>(res) : 32.f;
    case ELoaderType::OBJ :
    {
      if (loader_obj_->GetMaterials().empty())
        return {};

      assert(mat_num < loader_obj_->GetMaterials().size());
      if (resource_name == "specular_power")
        return loader_obj_->GetMaterials()[mat_num].specular[0]; // todo:
    }
  }
  return {};
}

template<class T>
inline std::vector<T> gdm::ModelLoader::LoadPositions(std::size_t mesh_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY :
      return ply::helpers::LoadXYZ<T>(*loader_ply_);
    case ELoaderType::OBJ :
      return obj::helpers::LoadXYZ<T>(*loader_obj_, mesh_num);
  }
  return {};
}

template<class T>
inline std::vector<T> gdm::ModelLoader::LoadIndicies(std::size_t mesh_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY :
      return ply::helpers::LoadFaces<T>(*loader_ply_);
    case ELoaderType::OBJ :
      return obj::helpers::LoadIndicies<T>(*loader_obj_, mesh_num);
  }
  return {};
}

template<class T>
inline std::vector<T> gdm::ModelLoader::LoadNormals(std::size_t mesh_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY :
      return ply::helpers::LoadNormals<T>(*loader_ply_);
    case ELoaderType::OBJ :
      return obj::helpers::LoadNormals<T>(*loader_obj_, mesh_num);
  }
  return {};
}

template<class T>
inline std::vector<T> gdm::ModelLoader::LoadUV(std::size_t mesh_num) const
{
  switch (type_)
  {
    case ELoaderType::PLY :
      return ply::helpers::LoadUV<T>(*loader_ply_);
    case ELoaderType::OBJ :
      return obj::helpers::LoadUV<T>(*loader_obj_, mesh_num);
  }
  return {};
}
