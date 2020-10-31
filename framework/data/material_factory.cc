// *************************************************************
// File:    material_factory.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "material_factory.h"

#include <system/hash_utils.h>
#include <system/assert_utils.h>
#include <system/array_utils.h>
#include <system/mesh_utils.h>

#include "helpers.h"
#include "model_loader.h"

#include "image_factory.h"
#include "texture_factory.h"

// --public

gdm::MaterialHandle gdm::MaterialFactory::Create(const char* mat_name, int mat_num, const ModelLoader& loader)
{
  using AVec4 = std::array<float,4>;

  ASSERTF(!Has(mat_name), "Trying to create already created material");

  AbstractMaterial* mat = GMNew AbstractMaterial{};

  std::unordered_map<const char*, TextureHandle*> map_name_to_handle {
    {"diffuse_map", &(mat->diff_)},
    {"normal_map", &(mat->norm_)},
    {"specular_map", &(mat->spec_)}
  };

  for (auto&& [tex_name, tex_handle] : map_name_to_handle)
  {
    std::string img_fname = loader.GetMaterial<std::string>(tex_name, mat_num);
    if (img_fname.empty())
      img_fname = std::string(ImageFactory::v_dummy_name) + tex_name;
    if (TextureFactory::Has(img_fname.c_str()))
      *tex_handle = TextureFactory::GetHandle(img_fname.c_str());
    else
      *tex_handle = TextureFactory::Load(img_fname.c_str());
  }

  mat->props_.emissive_ = Vec4f(&loader.GetMaterial<AVec4>("emissive", mat_num)[0]);
  mat->props_.ambient_ = Vec4f(&loader.GetMaterial<AVec4>("ambient", mat_num)[0]);
  mat->props_.diffuse_ = Vec4f(&loader.GetMaterial<AVec4>("diffuse", mat_num)[0]);
  mat->props_.specular_ = Vec4f(&loader.GetMaterial<AVec4>("specular", mat_num)[0]);
  mat->props_.specular_power_ = loader.GetMaterial<float>("specular_power", mat_num); 

  MaterialHandle handle = helpers::GenerateHandle(mat_name);
  resources_[handle] = mat;
  return handle;
}
