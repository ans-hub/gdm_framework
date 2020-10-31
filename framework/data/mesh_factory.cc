// *************************************************************
// File:    mesh_factory.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "mesh_factory.h"

#include <filesystem>
#include <vector>

#include <system/hash_utils.h>
#include <system/assert_utils.h>
#include <system/array_utils.h>
#include <system/mesh_utils.h>

#include "helpers.h"
#include "model_loader.h"
#include "abstract_image.h"

// --public

gdm::MeshHandle gdm::MeshFactory::Create(const char* mesh_name, int mesh_num, const ModelLoader& loader)
{
  ASSERTF(!Has(mesh_name), "Trying to create already created mesh");
  
  AbstractMesh* mesh = GMNew AbstractMesh {};

  const char* mesh_mat_name = loader.GetMeshMaterialName(mesh_num);
  if (*mesh_mat_name == '\000')
    mesh_mat_name = MaterialFactory::v_dummy_name;
  ASSERTF(MaterialFactory::Has(mesh_mat_name), "Trying to get not created material");
  MaterialHandle mat_handle = MaterialFactory::GetHandle(mesh_mat_name);
  mesh->material_ = mat_handle;

  mesh->faces_ = loader.LoadIndicies<Vec3u>(mesh_num);
  ASSERT(!mesh->faces_.empty());

  mesh->coords_ = loader.LoadPositions<Vec3f>(mesh_num);
  if (AbstractMesh::normalize_poses_)
    arr_utils::Normalize(&mesh->coords_[0].x, mesh->coords_.size() * 3, -1.f, 1.f);
  ASSERT(!mesh->coords_.empty());

  mesh->normals_ = loader.LoadNormals<Vec3f>(mesh_num);
  if (mesh->normals_.empty())
    mesh->normals_ = mesh_utils::ComputeVxsNormals(mesh->coords_, mesh->faces_);
  ASSERT(mesh->normals_.size() == mesh->coords_.size());

  mesh->texuv_ = loader.LoadUV<Vec2f>(mesh_num);
  if (!mesh->texuv_.empty())
  {
    ASSERT(mesh->coords_.size() == mesh->texuv_.size());
    std::vector<Mat4f> vxs_tbns = mesh_utils::ComputeVxsTangentSpace(mesh->coords_, mesh->texuv_, mesh->normals_, mesh->faces_);

    ASSERT(vxs_tbns.size() == mesh->coords_.size());
    std::vector<Vec3f> tangents {};

    for (const auto& tbn : vxs_tbns)
      mesh->tangents_.push_back(vec3::Normalize(tbn.GetCol(0)));
    ASSERT(mesh->tangents_.size() == mesh->coords_.size());
  }
  else // make fake uv coords and tangents for uniform shader access
  {
    mesh->texuv_.resize(mesh->coords_.size(), Vec2f{0.5f, 0.5f});
    mesh->tangents_.resize(mesh->coords_.size(), Vec3f{0.f, 1.f, 0.f});
  }

  AddToVxsBuffer(mesh, mesh->coords_);
  AddToVxsBuffer(mesh, mesh->texuv_);
  AddToVxsBuffer(mesh, mesh->normals_);
  AddToVxsBuffer(mesh, mesh->tangents_);
  
  MeshHandle handle = helpers::GenerateHandle(mesh_name);
  resources_[handle] = mesh;
  return handle;
}
