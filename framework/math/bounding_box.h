// *************************************************************
// File:    bounding_box.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_BOUNDING_BOX_H
#define AH_GDM_BOUNDING_BOX_H

#include <math/vector3.h>

namespace gdm {

struct AABB
{
  float w_;
  float h_;
  float d_;
  Vec3f world_pos_;

}; // struct AABB

struct OBB
{
  float axis_x_;
  float axis_y_;
  float axis_z_;
  float w_;
  float h_;
  float d_;
  Vec3f world_pos_;

}; // struct OBB

} // namespace gdm

#endif  // AH_GDM_BOUNDING_BOX_H
