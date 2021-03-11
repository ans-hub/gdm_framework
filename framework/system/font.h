// *************************************************************
// File:    font.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_FONT_H
#define AH_GDM_FONT_H

#include <string>
#include <vector>

#include "math/vector2.h"
#include "math/vector4.h"

namespace gdm {

struct Font
{
  struct Character
  {
    Vec4i coords_;
    Vec2i bearing_;
    int advance_;
  };

  struct Metrics
  {
    int font_height_;
    int texture_width_;
    int texture_height_;
  };

  struct Ops
  { };

  Font(const std::string& font_name, int size_pt, Ops ops = {});

  void Dump(const std::string& png_name);
  auto operator[](int i) const -> const Character& { return characters_[i]; }
  auto GetMetrics() const -> const Metrics& { return metrics_; }
  auto GetRaw() const -> const std::vector<unsigned char>& { return atlas_data_; }

private:
  constexpr static const int v_chars_cnt_ = 128;

  struct NativeFont;

private:
  void FillMetrics(NativeFont& nf);
  void LoadChar(NativeFont& ft, unsigned char ch);

private:
  std::vector<Character> characters_;
  Metrics metrics_;
  std::vector<unsigned char> atlas_data_;

}; // struct Font

} // namespace gdm

#endif  // AH_GDM_FONT_H