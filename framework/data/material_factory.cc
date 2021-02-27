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

#include "model_loader.h"
#include "image_factory.h"
#include "texture_factory.h"

// --public

gdm::MaterialHandle gdm::MaterialFactory::Create(const char* mat_name, int mat_num, const ModelLoader& loader)
{
  ASSERTF(!Has(mat_name), "Trying to create already created material");

  static uint s_index = 0;  

  AbstractMaterial* mat = GMNew AbstractMaterial{};
  mat->index_ = s_index++;

  mat->props_.emissive_ = Vec4f(&loader.GetMaterial<Vec4f>("emissive", mat_num)[0]);
  mat->props_.ambient_ = Vec4f(&loader.GetMaterial<Vec4f>("ambient", mat_num)[0]);
  mat->props_.diffuse_ = Vec4f(&loader.GetMaterial<Vec4f>("diffuse", mat_num)[0]);
  mat->props_.specular_ = Vec4f(&loader.GetMaterial<Vec4f>("specular", mat_num)[0]);
  mat->props_.specular_power_ = loader.GetMaterial<float>("specular_power", mat_num); 

  std::unordered_map<const char*, TextureHandle*> map_name_to_handle {
    {"diffuse_map", &(mat->diff_)},
    {"normal_map", &(mat->norm_)},
    {"specular_map", &(mat->spec_)}
  };

  for (auto&& [tex_name, tex_handle] : map_name_to_handle)
  {
    std::string img_fname = loader.GetMaterial<std::string>(tex_name, mat_num);
    if (img_fname.empty())
    {
      img_fname = std::string(ImageFactory::v_dummy_name) + tex_name;
      if (TextureFactory::Has(img_fname.c_str()))
        *tex_handle = TextureFactory::GetHandle(img_fname.c_str());
      else
      {
        if (strcmp(tex_name, "diffuse_map") == 0)
          *tex_handle = TextureFactory::Create(img_fname.c_str(), {32,32,32}, {mat->props_.diffuse_.r, mat->props_.diffuse_.g, mat->props_.diffuse_.b});
        else
          *tex_handle = TextureFactory::Create(img_fname.c_str(), {32,32,32}, {0,0,0});
      }
    }
    else
    {
      if (TextureFactory::Has(img_fname.c_str()))
        *tex_handle = TextureFactory::GetHandle(img_fname.c_str());
      else
        *tex_handle = TextureFactory::Load(img_fname.c_str());
    }
  }

  MaterialHandle handle = helpers::GenerateHandle(mat_name);
  resources_[handle] = mat;
  return handle;
}

bool gdm::MaterialFactory::ImplementationLoaded(MaterialHandle handle)
{
  ASSERTF(Has(handle), "Trying to request properties of already non loaded material");  

  bool loaded = false;
  AbstractMaterial* material = Get(handle);
  for (auto handle : material->GetTextureHandles())
  {
    if (TextureFactory::Has(handle))
      loaded |= TextureFactory::Get(handle)->image_impl_ && TextureFactory::Get(handle)->image_view_impl_;
  }
  return loaded;
}