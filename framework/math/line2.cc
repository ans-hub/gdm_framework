// *************************************************************
// File:    line2.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_math
// *************************************************************

#include "line2.h"

#include <cmath>

gdm::Line2 gdm::line2::EquationA(const Vec2f& p1, const Vec2f& p2)
{
  // todo: check if p1 == p2;
  Line2 res {};
  res.a = p1.y - p2.y;
  res.b = p2.x - p1.x;
  res.c = -res.a*p1.x - res.b*p1.y;
  return res;
}

// A(x-x0) + B(y-y0) + C = 0 for v(A;B) and p(x0,y0)

gdm::Line2 gdm::line2::EquationB(const Vec2f& p, const Vec2f& norm)
{
  Line2 res {};
  res.a = norm.x;
  res.b = norm.y;
  res.c = norm.x*(-p.x) + norm.y*(-p.y);
  return res;
}

gdm::Line2 gdm::line2::EquationC(const Vec2f& radius_v)
{
  return line2::EquationA(Vec2f(0,0), Vec2f(radius_v.x, radius_v.y));
}

gdm::Line2 gdm::line2::Perpendicular(const Line2& l, const Vec2f& p)
{
  Vec2f p1 {l.GetXAxisIntersection(0.0), 0.0};
  Vec2f p2 {0.0, l.GetYAxisIntersection(0.0)};
  Vec2f dirv (p2-p1);
  return line2::EquationB(p, dirv);
}

// Segments intersection using parametric form

bool gdm::line2::Intersects(const Seg2& l, const Seg2& r, Vec2f& res)
{
  Vec2f v1 {l.b - l.a};
  Vec2f v2 {r.b - r.a};
  
  float vmul = v1.x*v2.y - v1.y*v2.x;  // vmul as matrix determ
  if (std::fabs(vmul) < math::kEpsilon)
    return false;
  
  float t1 = ( v2.x*(l.a.y - r.a.y) - v2.y*(l.a.x - r.a.x) ) / vmul;
  float t2 = ( v1.x*(l.a.y - r.a.y) - v1.y*(l.a.x - r.a.x) ) / vmul;

  if ((t1 >= 0) && (t1 <= 1) && (t2 >= 0) && (t2 <= 1)){
    res.x = l.a.x + (t1 * v1.x);
    res.y = l.a.y + (t1 * v1.y); 
    return true;
  } 
  else
    return false;
}

bool gdm::line2::Intersects(const Line2& l1, const Line2& l2, Vec2f& p)
{
  float det = l1.a*l2.b - l1.b*l2.a;
  if (std::fabs(det) < math::kEpsilon)
    return false;
  p.x = -(l1.c*l2.b - l2.c*l1.b) / det;
  p.y = -(l1.a*l2.c - l2.a*l1.c) / det;
  return true;
}

bool gdm::line2::Intersects(const Line2& l1, const Seg2& s, Vec2f& p)
{
  Line2 l2 = line2::EquationA(s.a, s.b);
  if (line2::Intersects(l1,l2,p))
  {
    float xmax = math::Max(s.a.x, s.b.x);
    float xmin = math::Min(s.a.x, s.b.x);
    float ymax = math::Max(s.a.y, s.b.y);
    float ymin = math::Min(s.a.y, s.b.y);
    
    if ( (p.x < xmax || math::FEq(p.x, xmax)) &&
         (p.x > xmin || math::FEq(p.x, xmin)) &&
         (p.y < ymax || math::FEq(p.y, ymax)) &&
         (p.y > ymin || math::FEq(p.y, ymin)) )
        return true;
  }
  return false;
}
