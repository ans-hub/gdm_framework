// *************************************************************
// File:    line2.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_math
// *************************************************************

#ifndef AH_GDM_LINE2_H
#define AH_GDM_LINE2_H

#include <cassert>

#include "math/general.h"
#include "math/segment2.h"
#include "math/vector2.h"

namespace gdm {

// Line 2d using general form ax + by + c = 0, where a || b != 0

struct Line2
{  
  float a, b, c;
 
  constexpr Line2() : a{1.f}, b{1.f}, c{0.f} { }
  constexpr Line2(float pa, float pb, float pc) : a{pa}, b{pb}, c{pc} { }

  float GetXAxisIntersection(float y) const { return ((-b*y)-c) / a; }
  float GetYAxisIntersection(float x) const { return ((-a*x)-c) / b; }

}; // struct Line2

namespace line2
{
  Line2 EquationA(const Vec2f& p0, const Vec2f& p1);
  Line2 EquationB(const Vec2f& p, const Vec2f& normal);
  Line2 EquationC(const Vec2f& radius_vec);
  Line2 Perpendicular(const Line2& l, const Vec2f& );

  bool Intersects(const Seg2& s0, const Seg2& s1, Vec2f& intersect_p);
  bool Intersects(const Line2& l0, const Line2& l1, Vec2f& intersect_p);
  bool Intersects(const Line2& l, const Seg2& s, Vec2f& intersect_p);

} // namespace line2

} // namespace gdm

#endif  // AH_GDM_LINE2_H