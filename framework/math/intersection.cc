// *************************************************************
// File:    intersection.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "intersection.h"

#include <math/vector3.h>
#include <math/sphere.h>
#include <math/obb.h>

//-- public

gdm::Vec3f gdm::phys::FindClosestPoint(const OBB& obb, const Vec3f& point)
{
	Vec3f result = obb.tm_.GetCol(3);
	Vec3f dir = point - result;

	for (int i = 0; i < 3; ++i)
  {
    Vec3f axis = vec3::Normalize(obb.tm_.GetCol(i));
		float distance = vec3::DotProduct(dir, axis);

		if (distance > obb.half_sizes_[i])
			distance = obb.half_sizes_[i];
    else if (distance < -obb.half_sizes_[i])
			distance = -obb.half_sizes_[i];

		result = result + (axis * distance);
	}

	return result;
}

bool gdm::phys::IsIntersects(const Sphere& sphere, const OBB& obb, CollisionManifold& col)
{
  col.closest_point = phys::FindClosestPoint(obb, sphere.world_pos_);

  float dist_sq = vec3::SqLength(sphere.world_pos_ - col.closest_point);
  float radius_sq = sphere.radius_ * sphere.radius_;

  if (dist_sq >= radius_sq)
    return false;

  Vec3f collision_norm = vec3::Normalize(sphere.world_pos_ - col.closest_point);
  Vec3f external_point = sphere.world_pos_ - collision_norm * sphere.radius_;
  
  col.normal = collision_norm;
  col.penetration = vec3::Length(col.closest_point - external_point) * 2;
  col.contact_points.push_back(col.closest_point + (external_point - col.closest_point) * 0.5f);
  
  return true;
}
