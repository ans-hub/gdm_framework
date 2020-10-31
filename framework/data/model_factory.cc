// *************************************************************
// File:    model_factory.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "model_factory.h"

#include "model_loader.h"

// --public

gdm::ModelHandle gdm::ModelFactory::Load(const char* fpath)
{
  ASSERTF(!Has(fpath), "Model already loaded");

  std::string full_path = std::string(resources_path_) + std::string(fpath);
  ModelLoader model_loader {full_path.c_str(), MaterialFactory::GetPath()};
  AbstractModel* model = GMNew AbstractModel();
  
  for (int i = 0; i < model_loader.GetMaterialsCount(); ++i)
  {
    const char* mat_name = model_loader.GetMaterialName(i);
    if (*mat_name == '\000')
      mat_name = MaterialFactory::v_dummy_name;

    Handle mat_handle {};
    if (MaterialFactory::Has(mat_name))
      mat_handle = MaterialFactory::GetHandle(mat_name);
    else
      mat_handle = MaterialFactory::Create(mat_name, i, model_loader);
    model->materials_.push_back(mat_handle);
  }
  for (int i = 0; i < model_loader.GetMeshesCount(); ++i)
  {
    std::string mesh_name = model_loader.GetMeshName(i);
    if (mesh_name == "")
      mesh_name = helpers::GenerateName(fpath, i);
    Handle mesh_handle {};
    if (MeshFactory::Has(mesh_name.c_str()))
      mesh_handle = MeshFactory::GetHandle(mesh_name.c_str());
    else
      mesh_handle = MeshFactory::Create(mesh_name.c_str(), i, model_loader);
    model->meshes_.push_back(mesh_handle);
  }
  ModelHandle handle = helpers::GenerateHandle(fpath);
  resources_[handle] = model;
  return handle;
}
