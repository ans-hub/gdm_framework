// *************************************************************
// File:    polygon2.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "polygon2.h"

#include "math/general.h"
#include "math/vector2.h"
#include "math/line2.h"

float gdm::poly2::Square(const std::vector<Vec2f>& p)
{
  assert(p.size() >= 2);
  
  float summ {0};
  int sz = (int)p.size();
  for (int i = 0, j = sz-1; i < sz; j = i++) // i - next, j - prev
    summ += (p[j].x * p[i].y - p[i].x * p[j].y);
  return summ/2;
}

gdm::Vec2f gdm::poly2::Barycenter(const std::vector<Vec2f>& p, float sq)
{
  assert(p.size() >= 2);

  Vec2f res {};
  int sz = (int)p.size();
  for (int i = 0, j = sz-1; i < sz; j = i++)  // i - next, j - prev
  {
    float summ = (p[j].x * p[i].y - p[i].x * p[j].y);
    res.x += (p[j].x + p[i].x) * summ;
    res.y += (p[j].y + p[i].y) * summ;
  }
  res.x /= 6*sq;
  res.y /= 6*sq;
  return res;
}

// Based on:
// https://wrf.ecse.rpi.edu/Research/Short_Notes/pnpoly.html

bool gdm::poly2::PointInside(const std::vector<Vec2f>& poly, const Vec2f& p)
{
  int c = 0;
  int sz = (int)poly.size();
  for (int i=0, j=sz-1; i<sz; j=i++) // i - next, j - prev
  {
    Vec2f prev = poly[j];
    Vec2f curr = poly[i];
    float deltaX = prev.x - curr.x;
    float deltaY = prev.y - curr.y;
    float ray = ( deltaX * (p.y-curr.y) / deltaY ) + curr.x;
    bool pnt_beside = (curr.y > p.y) != (prev.y > p.y);

    if (pnt_beside && p.x < ray)
       c = !c;
  }
  return c == 0 ? false : true;
}

bool gdm::poly2::PointInside(float x0, float y0, float x1, float y1, float px, float py)
{
  if (px < x0 || px > x1 || py < y0 || py > y1)
    return false;
  return true;
}

bool gdm::poly2::PointInside(const Vec2f& p1, const Vec2f& p2, const Vec2f& p)
{
  if (p.x < p1.x || p.x > p1.x || p.y < p2.y || p.y > p2.y)
    return false;
  return true;
}

bool gdm::poly2::PointsInside(float x0, float y0, float x1, float y1, const std::vector<Vec2f>& v)
{
  for (const auto& p : v)
  {
    if (p.x < x0 || p.x > x1 || p.y < y0 || p.y > y1)
      return false;
  }
  return true;}

bool gdm::poly2::CutConvex(std::vector<Vec2f>& p1, std::vector<Vec2f>& p2, const Line2& l)
{
  p1.push_back(p1.front());
  std::vector<Vec2f> t1{};
  std::vector<Vec2f> t2{};
  bool in_two {false};
  
  for (auto it = p1.begin(); it != p1.end()-1; ++it)
  {  
    if (!in_two)
      t1.push_back(*it);
    else
      t2.push_back(*it);      
    
    Seg2 s(*it, *(it+1));
    Vec2f p{};
    if (line2::Intersects(l,s,p)) {
      t1.push_back(p);
      t2.push_back(p);
      in_two = !in_two;
    }
  }
  p1 = t1;
  p2 = t2;
  return true;
}

// Rotates point (by origin)

void gdm::poly2::RotatePoint(float& x, float& y, float theta)
{
  float sin_theta = std::sin(theta);;
  float cos_theta = std::cos(theta);
  x = x * cos_theta - y * sin_theta;
  y = y * sin_theta + y * cos_theta;
}
