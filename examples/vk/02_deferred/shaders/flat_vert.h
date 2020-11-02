// *************************************************************
// File:    flat_vert.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// PO - per-object
// PF - per-frame
// PI - per-instance
// CB - constant buffer

#include <math/matrix.h>
#include <math/vector3.h>
#include <math/vector4.h>

#ifndef SH_FLAT_VERT_DEF_H
#define SH_FLAT_VERT_DEF_H

namespace gdm {

struct alignas(64) FlatVs_PFCB
{
  alignas(16) Mat4f u_view_proj_;

}; // struct FlatVs_PFCB

struct alignas(64) FlatVs_POCB
{
  alignas(16) Mat4f u_model_;
  alignas(16) Vec4f u_color_;

}; // struct FlatVs_POCB

} // namespace gdm

#endif // SH_FLAT_VERT_DEF_H
