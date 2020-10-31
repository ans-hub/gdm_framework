// *************************************************************
// File:    desc_vert.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// PO - per-object
// PF - per-frame
// PI - per-instance
// CB - constant buffer

#include <math/matrix.h>
#include <math/vector3.h>

#ifndef AH_GDM_FLAT_VERT_SH_H
#define AH_GDM_FLAT_VERT_SH_H

namespace gdm
{
__declspec(align(16)) struct FlatVx_PFCB
{
  alignas(16) Mat4f mx_vp;
  alignas(16) Mat4f mx_light_tm;

}; // struct FlatVx_PFCB

__declspec(align(16)) struct FlatVx_POCB
{
  alignas(16) Mat4f mx_model;

}; // struct FlatVx_POCB

} // namespace gdm

#endif // AH_GDM_FLAT_VERT_SH_H
