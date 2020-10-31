// *************************************************************
// File:    desc_frag.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// PO - per-object
// PF - per-frame
// PI - per-instance
// CB - constant buffer

#include <math/matrix.h>
#include <math/vector4.h>

#ifndef SH_FLAT_FRAG_DEF_H
#define SH_FLAT_FRAG_DEF_H

namespace gdm
{
__declspec(align(16)) struct FlatPx_POCB
{
  Vec4f emissive_ = Vec4f(0.f);
  Vec4f ambient_ = Vec4f(0.1f);
  Vec4f diffuse_ = Vec4f(1.f);
  Vec4f specular_ = Vec4f(1.f);
  float specular_power_ = 128.f;

}; // struct FlatPx_PFCB

} // namespace gdm

#endif // SH_FLAT_FRAG_DEF_H
