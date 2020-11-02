// *************************************************************
// File:    dx_material.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_material.h"

#include <filesystem>

// --private

namespace gdm::_private {

struct TextureDescImpl : TextureDesc
{
  TextureDescImpl() : TextureDesc()
  {
    format_ = gfx::EFormatType::UNORM4;
    usage_ = gfx::EUsageType::IMMUTABLE;
    srv_dimension_ =  gfx::ESrvDimension::SRV_TEX2D;
    bind_type_ = gfx::EBindType::SRV;
  }

}; // struct TextureDescImpl

} // namespace gdm::_private

// --public

gdm::DxMaterial::DxMaterial()
  : name_{}
  , diffuse_map_{}
  , normal_map_{}
  , specular_map_{}
  , props_{}
{ }

gdm::DxMaterial::DxMaterial(ModelLoader& model, ID3D11Device* device, std::size_t mat_num)
  : name_{model.GetMaterialName(mat_num)}
  , diffuse_map_{}
  , normal_map_{}
  , specular_map_{}
  , props_{}
{
  namespace fs = std::filesystem;

  fs::path model_path {model.GetFilePath()};
 
  std::string img_fname = model.GetMaterial<std::string>("diffuse_map", mat_num);
  fs::path img_path = model_path.parent_path().concat("/" + img_fname);
  AbstractImage* img = helpers::MakeImage(img_path.string().c_str(), {1,1,1});
  diffuse_map_ = std::move(DxTexture(device, *img, _private::TextureDescImpl{}));

  img_fname = model.GetMaterial<std::string>("normal_map", mat_num);
  img_path = model_path.parent_path().concat("/" + img_fname);
  img = helpers::MakeImage(img_path.string().c_str(), {1,1,1});  // convient with ps as not normals
  normal_map_ = std::move(DxTexture(device, *img, _private::TextureDescImpl{}));

  img_fname = model.GetMaterial<std::string>("specular_map", mat_num);
  img_path = model_path.parent_path().concat("/" + img_fname);
  img = helpers::MakeImage(img_path.string().c_str());
  specular_map_ = std::move(DxTexture(device, *img, _private::TextureDescImpl{}));

  props_.emissive_ = Vec4f(&model.GetMaterial<Vec4f>("emissive", mat_num)[0]);
  props_.ambient_ = Vec4f(&model.GetMaterial<Vec4f>("ambient", mat_num)[0]);
  props_.diffuse_ = Vec4f(&model.GetMaterial<Vec4f>("diffuse", mat_num)[0]);
  props_.specular_ = Vec4f(&model.GetMaterial<Vec4f>("specular", mat_num)[0]);
  props_.specular_power_ = model.GetMaterial<float>("specular_power", mat_num);
}

gdm::DxMaterial::DxMaterial(DxMaterial&& mat)
  : name_{mat.name_}
  , diffuse_map_{std::move(mat.diffuse_map_)}
  , normal_map_{std::move(mat.normal_map_)}
  , specular_map_{std::move(mat.specular_map_)}
  , props_{mat.props_}
{ }

gdm::DxMaterial& gdm::DxMaterial::operator=(DxMaterial&& mat)
{
  if (this != &mat)
  {
    name_ = mat.name_;
    diffuse_map_ = std::move(mat.diffuse_map_);
    normal_map_ = std::move(mat.normal_map_);
    specular_map_ = std::move(mat.specular_map_);
    props_ = mat.props_;
  }
  return *this;
}
