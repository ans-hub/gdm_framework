// *************************************************************
// File:    dx_material.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_MATERIAL_H
#define AH_DX_MATERIAL_H

#include <d3d11.h>

#include <math/vector3.h>
#include <math/vector4.h>
#include <system/assert_utils.h>
#include <data/model_loader.h>
#include <data/cfg_loader.h>
#include <data/helpers.h>

#include "dx_texture.h"

namespace gdm {

struct DxMaterial
{
  __declspec(align(16)) struct Props
  {
    Vec4f emissive_ = Vec4f(0.f);
    Vec4f ambient_ = Vec4f(0.1f);
    Vec4f diffuse_ = Vec4f(1.f);
    Vec4f specular_ = Vec4f(1.f);
    float specular_power_ = 128.f;

  }; // struct Props

public:
  DxMaterial();
  DxMaterial(ModelLoader& model, ID3D11Device* device, std::size_t mat_num);

  DxMaterial(const DxMaterial& mat) =delete;
  DxMaterial& operator=(const DxMaterial& mat) =delete;
  DxMaterial(DxMaterial&& mat);
  DxMaterial& operator=(DxMaterial&& mat);

  auto GetName() const -> const std::string& { return name_; }

public:
  std::string name_;
  DxTexture diffuse_map_;
  DxTexture normal_map_;
  DxTexture specular_map_;
  alignas(16) Props props_;

}; // struct DxMaterial

} // namespace gdm

#endif // AH_DX_MATERIAL_H