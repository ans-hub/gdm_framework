// *************************************************************
// File:    font.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "font.h"

#include "system/assert_utils.h"

gdm::Font::Font(const char* font_name, int font_size)
{
  [[maybe_unused]] bool success = false;

  FT_Library ft;
  success = FT_Init_FreeType(&ft);
  ASSERTF(success, "Could not init FreeType Library");

  FT_Face face;
  success = FT_New_Face(ft, font_name, 0, &face);
  ASSERTF(success, "Failed to load font");

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}