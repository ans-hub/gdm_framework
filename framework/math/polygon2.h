// *************************************************************
// File:    polygon2.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_POLY2_H
#define AH_GDM_POLY2_H

#include <vector>

#include "math/polygon2.h"
#include "math/line2.h"
#include "math/vector2.h"

namespace gdm {

namespace poly2 {

  float Square(const std::vector<Vec2f>& p);
  Vec2f Barycenter(const std::vector<Vec2f>& p, float sq);
  bool PointInside(const std::vector<Vec2f>& p, const Vec2f&);
  bool PointInside(float x0, float y0, float x1, float y1, float px, float py);
  bool PointInside(const Vec2f& p1, const Vec2f& p2, const Vec2f& p0);
  bool PointsInside(float x0, float y0, float x1, float y1, const std::vector<Vec2f>& p);
  bool CutConvex(std::vector<Vec2f>& p1, std::vector<Vec2f>& p2, const Line2& l);
  void RotatePoint(float&, float&, float theta);

} // namespace poly2

} // namespace gdm

#endif  // AH_GDM_POLY2_H