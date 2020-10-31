// *************************************************************
// File:    line3.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "line3.h"

gdm::Line3::Line3(const Vec3f& p0, const Vec3f& p1)
  : p0{p0}
  , p1{p1}
  , dx{p1.x - p0.x}
  , dy{p1.y - p0.y}
  , dz{p1.z - p0.z}
{ }

gdm::Vec3f gdm::line3::Intersects(const Line3& l, const Plane3& p)
{
  Vec3f res{};
  
  // Substitute unknowns in plane equation to parametric components of line
  //  plane: ax + by + cz - d = 0
  //  substitute: a*(x0+dx*t) + b(y0+dy*t) + c(z0+dz*t) - d = 0; 

  // Open braces, collect free members and members contains `t`, and find `t`

  float tfree = p.a * l.p0.x + p.b * l.p0.y + p.c * l.p0.z + p.d;
  float tcont = p.a * l.dx + p.b * l.dy + p.c * l.dz;
  float t = -tfree / tcont;

  // Find point which intersects plane and line
  
  res.x = l.p0.x + l.dx*t;
  res.y = l.p0.y + l.dy*t;
  res.z = l.p0.z + l.dz*t;

  return res;
}

// The same as above but returns intersects point through argument and bool as
// result of searching intersects point
//  todo: additional check to t-range in 0-1

bool gdm::line3::Intersects(const Line3& l, const Plane3& p, Vec3f& res)
{
  Vec3f dir    {l.dx, l.dy, l.dz};
  Vec3f normal {0.0f, 0.0f, 1.0f};
  auto prod = vec3::DotProduct(dir, normal);
  
  if (math::FNotZero(prod)) {
    res = line3::Intersects(l, p);
    return true;
  }
  else
    return false;
}

// Returns 't' parameter with which line and plane are would intersects

float gdm::line3::FindIntersectsT(const Line3& l, const Plane3& p)
{
  float tfree = p.a * l.p0.x + p.b * l.p0.y + p.c * l.p0.z + p.d;
  float tcont = p.a * l.dx + p.b * l.dy + p.c * l.dz;
  return -tfree / tcont;
}
