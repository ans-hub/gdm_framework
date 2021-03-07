// *************************************************************
// File:    font.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "font.h"

#include <filesystem>

#include "system/assert_utils.h"

gdm::Font::Font(const std::string& font_name, int font_size)
{
  [[maybe_unused]] int success = -1;

  FT_Library ft;
  success = FT_Init_FreeType(&ft);
  ASSERTF(success == 0, "Could not init FreeType Library (%d)", success);

  std::string font_path {"/" + font_name}; 
  font_path = std::filesystem::current_path().concat(font_path).string();

  FT_Face face;
  success = FT_New_Face(ft, font_path.c_str(), 0, &face);
  ASSERTF(success == 0, "Failed to load font (%d)", success);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}