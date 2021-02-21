// *************************************************************
// File:    debug_draw.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "debug_draw.h"

#include <math/obb.h>
#include <math/aabb.h>
#include <math/sphere.h>
#include <math/general.h>

//--public

void gdm::DebugDraw::DrawCross(Vec3f point, float len, Vec4f color)
{
  Vec3f r (1.f, 0.f, 0.f);
  Vec3f u (0.f, 1.f, 0.f);
  Vec3f f (0.f, 0.f, 1.f);

  data_.push_back({point, color});
  data_.push_back({point + r * len, color});
  data_.push_back({point, color});
  data_.push_back({point + r * -len, color});
  data_.push_back({point, color});
  data_.push_back({point + u * len, color});
  data_.push_back({point, color});
  data_.push_back({point + u * -len, color});
  data_.push_back({point, color});
  data_.push_back({point + f * len, color});
  data_.push_back({point, color});
  data_.push_back({point + f* - len, color});
}

void gdm::DebugDraw::DrawLine(Vec3f begin, Vec3f end, Vec4f color)
{
  data_.push_back({begin, color});
  data_.push_back({end, color});
}

void gdm::DebugDraw::DrawBasis(Vec3f wpos, Vec3f right, Vec3f up, Vec3f fwd, float len)
{
  data_.push_back({wpos, color::LightRed});
  data_.push_back({wpos + up * len, color::LightRed});
  data_.push_back({wpos, color::LightGreen});
  data_.push_back({wpos + right * len, color::LightGreen});
  data_.push_back({wpos, color::LightBlue});
  data_.push_back({wpos + fwd * len, color::LightBlue});
}

void gdm::DebugDraw::DrawBasis(const Mat4f& mx, float len)
{
  DrawBasis(mx.GetCol(3), mx.GetCol(0), mx.GetCol(1), mx.GetCol(2), len);
}

void gdm::DebugDraw::DrawBox(const Vec3f& wpos, const Vec3f& half_sizes, Vec4f color)
{
  float x = wpos.x;
  float y = wpos.y;
  float z = wpos.z;
  float w = half_sizes.w;
  float h = half_sizes.h;
  float d = half_sizes.d;
  
  Vec3f va (x-w, y-h, z-d);
  Vec3f vb (x-w, y-h, z+d);
  Vec3f vc (x-w, y+h, z-d);
  Vec3f vd (x-w, y+h, z+d);
  Vec3f ve (x+w, y-h, z-d);
  Vec3f vf (x+w, y-h, z+d);
  Vec3f vg (x+w, y+h, z-d);
  Vec3f vh (x+w, y+h, z+d);
  
  DrawLine(va, vb, color);
  DrawLine(vb, vd, color);
  DrawLine(vd, vc, color);
  DrawLine(vc, va, color);
  
  DrawLine(ve, vf, color);
  DrawLine(vf, vh, color);
  DrawLine(vh, vg, color);
  DrawLine(vg, ve, color);

  DrawLine(va, ve, color);
  DrawLine(vb, vf, color);
  DrawLine(vd, vh, color);
  DrawLine(vc, vg, color);
}

void gdm::DebugDraw::DrawSphere(const Vec3f& wpos, float radius, Vec4f color)
{
  static constexpr int v_step = 15;
  static constexpr int v_parts = 360 / v_step;

  Vec3f rvec(0.f, 0.f, radius);
  
  std::array<Vec3f, v_parts> cache;
  cache.fill(wpos + rvec);

  for (int i = v_step; i <= 360; i += v_step)
  {
      const float s = std::sin(trig::Deg2rad(i));
      const float c = std::cos(trig::Deg2rad(i));

      Vec3f last_point (wpos.x, wpos.y + radius * s, wpos.z + radius * c);

      for (int n = 0, j = v_step; j <= 360; j += v_step, ++n)
      {
        const float js = std::sin(trig::Deg2rad(j));
        const float jc = std::cos(trig::Deg2rad(j));

        Vec3f aux(wpos.x + radius * s * js, wpos.y + radius * s * jc, last_point.z);

        DrawLine(last_point, aux, color);
        DrawLine(last_point, cache[n], color);

        cache[n] = last_point;
        last_point = aux;
      }
  }
}