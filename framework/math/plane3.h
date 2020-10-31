// *************************************************************
// File:    plane3.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_PLANE3_H
#define AH_GDM_PLANE3_H

namespace gdm {

// Plane 3d as equation ax + by + cz + d = 0

struct Plane3
{
  Plane3(float ka, float kb, float kc, float kd) : a{ka}, b{kb}, c{kc}, d{kd} { }

  float a, b, c, d;

}; // struct Plane3

// Todo:  add parametric equation for plane
//        add plane/plane intersection with line eq as result

} // namespace gdm

#endif  // AH_GDM_PLANE3_H