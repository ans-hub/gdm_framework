// *************************************************************
// File:    debug_draw.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DEBUG_DESC_H
#define AH_DEBUG_DESC_H

#include <vector>

#include <math/vector3.h>
#include <math/matrix.h>
#include <render/colors.h>

namespace gdm {

struct AABB;
struct OBB;
struct Sphere;

struct DebugData
{
  Vec3f pos_;
  Vec4f color_;

}; // struct DebugData

struct DebugDraw
{
  void DrawCross(Vec3f point, float len, Vec4f color = color::LightGray);
  void DrawLine(Vec3f begin, Vec3f end, Vec4f color = color::LightGray);
  void DrawBasis(Vec3f wpos, Vec3f right, Vec3f up, Vec3f fwd, float len = 1.f);
  void DrawBasis(const Mat4f& mx, float len = 1.f);
  void DrawBox(const Vec3f& wpos, const Vec3f& half_sizes, Vec4f color = color::LightGray);
  void DrawSphere(const Vec3f& wpos, float radius, Vec4f color = color::LightGray);

  auto GetData() const -> std::vector<DebugData> { return data_; } 
  void Clear() { data_.clear(); }

  bool IsActive() const { return is_active_; }
  void ToggleActive() { is_active_ ^= true; }

private:
  std::vector<DebugData> data_ = {};
  bool is_active_ = false;

}; // struct DebugDraw

} // namespace gdm

#endif // AH_DEBUG_DESC_H
