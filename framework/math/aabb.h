// *************************************************************
// File:    aabb.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_AABB_H
#define AH_GDM_AABB_H

#include <math/vector3.h>
#include <math/matrix.h>

namespace gdm {

struct AABB
{
	Vec3f min_;
	Vec3f max_;

}; // struct AABB

} // namespace gdm::phys

#endif  // AH_GDM_AABB_H
