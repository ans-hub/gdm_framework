// *************************************************************
// File:    model_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MODEL_LOADER_H
#define AH_GDM_MODEL_LOADER_H

#include <vector>
#include <string>

#include "cfg_loader.h"
#include "ply_loader.h"
#include "obj_loader.h"

namespace gdm {

struct ModelLoader
{
  ModelLoader(const char* model_path, const char* texture_path);

  auto GetFilePath() const -> const char*;
  auto GetMeshesCount() const -> int;
  auto GetMaterialsCount() const -> int;
  auto GetMaterialName(std::size_t mat_num) const -> const char*;
  auto GetMeshName(std::size_t mesh_num) const -> const char*;
  auto GetMeshMaterialName(std::size_t mesh_num) const -> const char*;

public:  
  template<class T>
  auto GetMaterial(const char* resource_name, std::size_t mesh_num = 0) const -> T;
  template<class T>
  auto LoadPositions(std::size_t mesh_num) const -> std::vector<T>;
  template<class T>
  auto LoadIndicies(std::size_t mesh_num) const -> std::vector<T>;
  template<class T>
  auto LoadNormals(std::size_t mesh_num) const -> std::vector<T>;
  template<class T>
  auto LoadUV(std::size_t mesh_num) const -> std::vector<T>;

private:
  enum ELoaderType : unsigned
  {
    PLY,
    OBJ
  }; // enum ELoaderType

private:
  std::string model_fname_;
  std::string material_path_;
  ELoaderType type_;
  std::unique_ptr<ply::Loader> loader_ply_;
  gdm::Config material_ply_;
  std::unique_ptr<obj::Loader> loader_obj_;

  inline static const char* v_dummy_name = "";

}; // struct ModelLoader

} // namespace gdm

#include "model_loader.inl"

#endif // AH_GDM_MODEL_LOADER_H
