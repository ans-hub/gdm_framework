// *************************************************************
// File:    line3.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_LINE3_H
#define AH_GDM_LINE3_H

#include "math/general.h"
#include "math/vector3.h"
#include "math/plane3.h"

namespace gdm {
  
// Line using parametric form: ax + by + cz + d = 0

struct Line3
{  
  Vec3f p0, p1;
  float dx, dy, dz;

  Line3(const Vec3f& p0, const Vec3f& p1);

  float EvaluateX(float t) { return p0.x + dx*t; }
  float EvaluateY(float t) { return p0.y + dy*t; }
  float EvaluateZ(float t) { return p0.z + dz*t; }

}; // struct PLine3

namespace line3
{
  Vec3f Intersects(const Line3&, const Plane3&);
  bool  Intersects(const Line3&, const Plane3&, Vec3f& result);
  float FindIntersectsT(const Line3&, const Plane3&);

} // namespace line3

} // namespace gdm

#endif  // AH_GDM_LINE3_H