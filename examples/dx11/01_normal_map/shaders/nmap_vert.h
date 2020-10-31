// *************************************************************
// File:    nmap_vert.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// PO - per-object    - sets by objects
// PF - per-frame     - sets by shader
// PI - per-instance
// CB - constant buffer

#include <math/matrix.h>
#include <math/vector3.h>

#ifndef SH_NMAP_VERT_DEF_H
#define SH_NMAP_VERT_DEF_H

#define NMAP_VERT_LIGHTS_CNT 4

namespace gdm
{
__declspec(align(16)) struct LightVs_PFCB
{
  alignas(16) Vec4f vector_WS_ = {}; // pos or dir
  alignas(16) int type_ = {};

}; // struct LightVs_PFCB

__declspec(align(16)) struct NmapVs_PFCB
{
  alignas(16) Mat4f u_view_proj_ = Mat4f(1.f);  // 64
  alignas(16) Vec4f u_cam_pos_WS_ = Vec4f(0.f); // +16 = 80
  alignas(16) LightVs_PFCB u_lights_[NMAP_VERT_LIGHTS_CNT] = {};

}; // struct NmapVs_PFCB

__declspec(align(16)) struct NmapVs_POCB
{
  alignas(16) Mat4f u_model_;

}; // struct NmapVs_POCB

} // namespace gdm

#endif // SH_NMAP_VERT_DEF_H
