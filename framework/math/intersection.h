// *************************************************************
// File:    intersection.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_INTERSECTION_H
#define AH_GDM_INTERSECTION_H

#include <vector>

#include <math/vector3.h>
#include <math/obb.h>
#include <math/sphere.h>

namespace gdm::phys {

  struct CollisionManifold
  {
    Vec3f normal = {};
    float penetration = 0.f;
    Vec3f closest_point = {};
    std::vector<Vec3f> contact_points = {};
  };

  auto FindClosestPoint(const OBB& obb, const Vec3f& point) -> Vec3f;
  bool IsIntersects(const Sphere& sphere, const OBB& obb, CollisionManifold& col);

} // namespace gdm::phys

#endif  // AH_GDM_INTERSECTION_H
