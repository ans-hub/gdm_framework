// *************************************************************
// File:    obb.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_OBB_H
#define AH_GDM_OBB_H

#include <math/vector3.h>
#include <math/matrix.h>

namespace gdm {

struct OBB
{
	Vec3f half_size_;
	const Mat4f& tm_;

}; // struct OBB

} // namespace gdm::phys

#endif  // AH_GDM_OBB_H
