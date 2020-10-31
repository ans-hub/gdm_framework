// *************************************************************
// File:    segment2.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "segment2.h"

#include "math/vector2.h"
#include "math/general.h"

gdm::Vec2f gdm::seg2::Divpoint(const Seg2& s, float ratio)
{
  return Vec2f(
    s.b.x - (s.b.x - s.a.x) * ratio,
    s.b.y - (s.b.y - s.a.y) * ratio
  );
}

// Clips the segment using Liang-Barsky algorithm
// Returns false, if line is outside the clipping area

// We have P1(x1,y1) and P2(x2,y2)
// The parametrized equation is:
//  x = x1 + vx*t
//  y = y1 + vy*t
// where  {x1,y1} - any point on the line (in our case is start point of the segment)
//        {vx,vy} - directing vector of the line (in our case is P2-P1)
//        t - any rational value (if from 0 to 1, the point lies on segment P1-P2)

bool gdm::seg2::Clip(int xmin, int ymin, int xmax, int ymax, int& x1, int& y1, int& x2, int& y2)
{
  enum Sides { LEFT, RIGHT, BOTTOM, TOP};

  int vx = x2 - x1;   // x component of directing vector
  int vy = y2 - y1;   // y component of directing vector
  float tmin = 1.0f;  // with parametr t1 = 0 we have p1
  float tmax = 0.0f;  // with parametr t2 = 1 we have p2
  
  int p;              // inequalities variables
  int q; 
  float t;

  // Traverse through sides of clippint rect

  for (Sides s = LEFT; s <= TOP; s = (Sides)(s+1))
  {
    switch (s)
    {
      case LEFT   : p = -vx; q = x1 - xmin; break;
      case RIGHT  : p = vx;  q = xmax - x1; break;
      case BOTTOM : p = -vy; q = y1 - ymin; break;
      case TOP    : p = vy;  q = ymax - y1; break;
    }
    t = static_cast<float>(q)/static_cast<float>(p);

    // p == 0 - line parallel boundary line
    // q < 0  - line outside boundary line
    // p < 0  - line proceeds outside to inside (and vice verca)
    // p !=0  - then r is intersection point

    if (p < 0)
      tmax = math::Max(tmax, t);
    else if (p > 0)
      tmin = math::Min(tmin, t);
    else if (q < 0)           // parallel and outside
      return false;
    
    if (tmax > tmin) {
      return false;
    }
  }

  int x1_res = x1 + (int)(tmax * (float)vx);
  int y1_res = y1 + (int)(tmax * (float)vy);
  x2 = x1 + (int)(tmin * (float)vx),
  y2 = y1 + (int)(tmin * (float)vy);
  x1 = x1_res;
  y1 = y1_res;

  return true;
}

// Overloaded version of clip function above with float reference arguments

bool gdm::seg2::Clip(int xmin, int ymin, int xmax, int ymax, float& x1, float& y1, float& x2, float& y2)
{
  int x1_ = (int)x1;
  int y1_ = (int)y1;
  int x2_ = (int)x2;
  int y2_ = (int)y2;

  bool result = seg2::Clip(xmin, ymin, xmax, ymax, x1_, y1_, x2_, y2_);
  
  x1 = (float)x1_;
  y1 = (float)y1_;
  x2 = (float)x2_;
  y2 = (float)y2_;
  
  return result;
}
