// *************************************************************
// File:    sphere.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_SPHERE_H
#define AH_GDM_SPHERE_H

#include <math/vector3.h>
#include <math/matrix.h>

namespace gdm {

struct Sphere
{
  float radius_;
	const Vec3f& world_pos_;

}; // struct Sphere

} // namespace gdm

#endif  // AH_GDM_SPHERE_H