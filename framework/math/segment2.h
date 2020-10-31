// *************************************************************
// File:    segment2.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_SEGMENT2_H
#define AH_GDM_SEGMENT2_H

#include "math/vector2.h"

namespace gdm {

struct Seg2
{
  Vec2f a, b;

  Seg2() = default;
  Seg2(const Vec2f& pa, const Vec2f& pb) : a{pa}, b{pb} { }

}; // struct Seg2

namespace seg2 {

  auto Divpoint(const Seg2& seg, float ratio) -> Vec2f;
  bool Clip(int xmin, int ymin, int xmax, int ymax, int& x1, int& y1, int& x2, int& y2);
  bool Clip(int xmin, int ymin, int xmax, int ymax, float& x1, float& y1, float& x2, float& y2);

} // namespace seg2

} // namespace gdm

#endif  // AH_GDM_SEGMENT2_H