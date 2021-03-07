// *************************************************************
// File:    font.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_FONT_H
#define AH_GDM_FONT_H

#include <string>
#include <vector>

#include "ft2build.h"
#include "freetype/freetype.h"

#include "math/vector2.h"

namespace gdm {

struct Font
{
  struct Character {
    Vec2i size_;
    Vec2i bearing_;
    unsigned advance_;
  };

  Font(const std::string& font_name, int font_size);

private:
  constexpr static const int v_chars_cnt_ = 128;

private:
  std::vector<Character> characters_;

}; // struct Font

} // namespace gdm

#endif  // AH_GDM_FONT_H