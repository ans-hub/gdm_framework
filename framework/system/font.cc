// *************************************************************
// File:    font.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "font.h"

#include <filesystem>

#include "system/assert_utils.h"

gdm::Font::Font(const std::string& font_name, int font_size)
  : characters_(v_chars_cnt_)
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

  success = FT_Set_Pixel_Sizes(face, 0, 48);
  ASSERTF(success == 0, "Failed to set pixel sizes (%d)", success);

  for (unsigned char c = 0; c < static_cast<unsigned char>(v_chars_cnt_); c++)
  {
    success = FT_Load_Char(face, c, FT_LOAD_RENDER);
    ASSERTF(success == 0, "Failed to load characher %s (%d)", c, success);

    Character character {
      Vec2i(face->glyph->bitmap.width, face->glyph->bitmap.rows),
      Vec2i(face->glyph->bitmap_left, face->glyph->bitmap_top),
      static_cast<unsigned int>(face->glyph->advance.x) };
  
    characters_[static_cast<int>(c)] = character;
  };

  FT_Done_Face(face);
  FT_Done_FreeType(ft);
}