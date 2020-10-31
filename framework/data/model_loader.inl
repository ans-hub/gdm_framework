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
inline std::array<float,4> gdm::ModelLoader::GetMaterial(const char* resource_name, std::size_t mat_num) const
{
  using V4 = std::array<float,4>;

  std::string res {resource_name};

  switch(type_)
  {
    case ELoaderType::PLY :
      return material_ply_.Has<V4>(res) ? material_ply_.Get<V4>(res) : V4{0,0,0,0};
    case ELoaderType::OBJ :
    {
      assert(mat_num < loader_obj_->LoadedMaterials.size());
      const obj::Material& material = loader_obj_->LoadedMaterials[mat_num];
      if (res == "emissive")
        return V4{material.Ke.X, material.Ke.Y, material.Ke.Z, 0.f};  // needs to be zero for shader notify
      if (res == "ambient")
        return V4{material.Ka.X, material.Ka.Y, material.Ka.Z, 1.f};
      if (res == "diffuse")
        return V4{material.Kd.X, material.Kd.Y, material.Kd.Z, 1.f};
      if (res == "specular")
        return V4{material.Ks.X, material.Ks.Y, material.Ks.Z, 1.f};
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
      assert(mat_num < loader_obj_->LoadedMaterials.size());
      const obj::Material& material = loader_obj_->LoadedMaterials[mat_num];

      if (strcmp(resource_name, "diffuse_map") == 0)
        return material.map_Kd;
      if (strcmp(resource_name, "specular_map") == 0)
        return material.map_Ks;
      if (strcmp(resource_name, "normal_map") == 0)
        return material.map_bump;
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
      assert(mat_num < loader_obj_->LoadedMaterials.size());
      if (resource_name == "specular_power")
        return loader_obj_->LoadedMaterials[mat_num].Ns;
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
