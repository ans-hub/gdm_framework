// *************************************************************
// File:    debug_draw.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DEBUG_DESC_H
#define AH_DEBUG_DESC_H

#include <vector>
#include <string>

#include <math/vector3.h>
#include <math/matrix.h>
#include <render/colors.h>
#include <render/renderer.h>
#include <system/font.h>

#include "engine/gpu_streamer.h"

namespace gdm {

struct AABB;
struct OBB;
struct Sphere;

struct DebugData
{
  Vec3f pos_;
  Vec4f color_;

}; // struct DebugData

struct TextData
{
  Vec3f pos_;
  Vec4f color_;
  std::string text_;

}; // struct TextData

struct DebugDraw
{
  ~DebugDraw();

  void Cleanup();

  void AddFont(GpuStreamer& gpu_streamer, const std::string& font_path, int size_pt);

  void DrawString(Vec3f pos, const std::string& text, Vec4f color);
  void DrawCross(Vec3f point, float len, Vec4f color = color::LightGray);
  void DrawLine(Vec3f begin, Vec3f end, Vec4f color = color::LightGray);
  void DrawBasis(Vec3f wpos, Vec3f right, Vec3f up, Vec3f fwd, float len = 1.f);
  void DrawBasis(const Mat4f& mx, float len = 1.f);
  void DrawBox(const Vec3f& wpos, const Vec3f& half_sizes, Vec4f color = color::LightGray);
  void DrawSphere(const Vec3f& wpos, float radius, Vec4f color = color::LightGray);

  auto GetDrawData() const -> std::vector<DebugData> { return draw_data_; } 
  auto GetTextData() const -> std::vector<TextData> { return text_data_; }
  auto GetFont() const -> const Font* { return font_.get(); }
  auto GetFontView() const -> const api::ImageView* { return font_view_; }
  void Update() { Clear(); }
  void Clear() { draw_data_.clear(); text_data_.clear(); }

  bool IsActiveWire() const { return is_active_wire_; }
  bool IsActiveText() const { return is_active_text_; }
  void ToggleActivateWire() { is_active_wire_ ^= true; }
  void ToggleActivateText() { is_active_text_ ^= true; }

private:
  std::vector<DebugData> draw_data_ = {};
  std::vector<TextData> text_data_ = {};
  std::unique_ptr<Font> font_ = nullptr;
  api::ImageView* font_view_ = nullptr;

  bool is_active_wire_ = false;
  bool is_active_text_ = false;

}; // struct DebugDraw

} // namespace gdm

#endif // AH_DEBUG_DESC_H
