// *************************************************************
// File:    phong_vert.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// PO - per-object    - sets by objects
// PF - per-frame     - sets by shader
// PI - per-instance
// CB - constant buffer

#include <math/matrix.h>
#include <math/vector3.h>

#include <render/dx11/dx_material.h>

#ifndef SH_PHONG_VERT_DEF_H
#define SH_PHONG_VERT_DEF_H

namespace gdm
{
__declspec(align(16)) struct Phong_PFCB
{
  alignas(16) Mat4f u_view_proj_;
  alignas(16) Vec4f u_cam_pos_WS_;

}; // struct Phong_PFCB

__declspec(align(16)) struct Phong_POCB
{
  alignas(16) Mat4f u_model_;

}; // struct Phong_POCB

static Phong_POCB g_phong_POCB {};
static Phong_PFCB g_phong_PFCB {};

} // namespace gdm

#endif // SH_PHONG_VERT_DEF_H
