// *************************************************************
// File:    nmap_frag.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// PO - per-object    - sets by objects
// PF - per-frame     - sets by shader
// PI - per-instance
// CB - constant buffer

#include <math/matrix.h>
#include <math/vector3.h>
#include <math/vector4.h>

#include <render/dx11/dx_uniform.h>
#include <render/colors.h>

#ifndef SH_NMAP_FRAG_DEF_H
#define SH_NMAP_FRAG_DEF_H

#define NMAP_FRAG_LIGHTS_CNT 4

namespace gdm{

enum LightType : int
{
  DIR, POINT 
};

__declspec(align(16)) struct LightPs
{
  Vec4f pos_ = Vec4f(0.f);
  Vec4f dir_ = Vec4f(0.f);
  Vec4f color_ = color::Black;
  float spot_angle_ = 0.f;
  float attenuation_const_ = 1.f;
  float attenuation_linear_ = 0.08f;
  float attenuation_quadr_ = 0.0f;
  LightType type_ = LightType::DIR;
  bool  enabled_ = false;

}; // struct LightPs

__declspec(align(16)) struct NmapPs_PFCB
{
  alignas(16) Vec4f global_ambient_ = Vec4f(0.2f);
  alignas(16) Vec4f camera_pos_ = Vec4f(0.f);
  alignas(16) std::array<LightPs,4> lights_ = {};

}; // struct NmapPs_PFCB

__declspec(align(16)) struct MaterialPs
{
  Vec4f emissive_ = Vec4f(0.f);
  Vec4f ambient_ = Vec4f(0.1f);
  Vec4f diffuse_ = Vec4f(1.f);
  Vec4f specular_ = Vec4f(1.f);
  float specular_power_ = 128.f;

}; // struct MaterialPs

__declspec(align(16)) struct NmapPs_POCB
{
  alignas(16) MaterialPs material_props_ = {};

}; // struct NmapPs_POCB

} // namespace gdm

#endif // SH_NMAP_FRAG_DEF_H
