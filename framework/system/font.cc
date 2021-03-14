// *************************************************************
// File:    font.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "font.h"

#include <stdio.h>
#include <filesystem>

#include "ft2build.h"
#include "freetype/freetype.h"

#include "math/vector4.h"
#include "system/assert_utils.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rdparty/stb/stb_image_write.h"

// x bearing (left bearing)	
// The left-side bearing, i.e., the horizontal distance from the current pen position to the left border of the glyph bitmap.

// y bearing (top bearing)
// The top-side bearing, i.e., the vertical distance from the current pen position to the top border of the glyph bitmap. This distance is positive for upwards y!

//--private

struct gdm::Font::NativeFont
{
  NativeFont(const std::string& path, int size_pt)
  {
    success_ = FT_Init_FreeType(&ft_);
    ASSERTF(success_ == 0, "Could not init FreeType Library (%d)", success_);

    success_ = FT_New_Face(ft_, path.c_str(), 0, &face_);
    ASSERTF(success_ == 0, "Failed to load font (%d)", success_);

    const int size_px = int(float(size_pt) * 1.333f);

    success_ = FT_Set_Pixel_Sizes(face_, 0, size_px);
    ASSERTF(success_ == 0, "Failed to set pixel sizes (%d)", success_);
  }

  ~NativeFont()
  {
    FT_Done_Face(face_);
    FT_Done_FreeType(ft_);
  }

  void LoadChar(unsigned char ch)
  {
    [[maybe_unused]] const int success = FT_Load_Char(face_, ch, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
    ASSERTF(success == 0, "Failed to load characher %u (%d)", ch, success);
  }

  FT_Library ft_;
  FT_Face face_;
  int success_ = -1;

}; // struct gdm::Font::NativeFont

//--private

void gdm::Font::FillMetrics(NativeFont& ft)
{
	int max_dim = (1 + int(ft.face_->size->metrics.height >> 6)) * int(ceilf(sqrtf(v_chars_cnt_)));
	int tex_width = 1;
	while(tex_width < max_dim) tex_width <<= 1;

  metrics_.texture_width_ = tex_width;
  metrics_.texture_height_ = tex_width;
  metrics_.font_height_ = ft.face_->size->metrics.height >> 6;
}

//--public

gdm::Font::Font(const std::string& font_name, int size_pt, Ops ops)
  : characters_(v_chars_cnt_)
  , metrics_{}
  , atlas_data_{}
{
  std::string font_path {"/" + font_name}; 
  font_path = std::filesystem::current_path().concat(font_path).string();

  NativeFont ft(font_name, size_pt);
  FillMetrics(ft);

  atlas_data_.resize(metrics_.texture_width_ * metrics_.texture_height_, 255);
  float recip_width = 1.0f / metrics_.texture_width_;
  float recip_height = recip_width;

	int pen_x = 0;
  int pen_y = 0;

  for (unsigned char c = v_first_char_; c < static_cast<unsigned char>(v_chars_cnt_); c++)
  {
    ft.LoadChar(c);

		FT_Bitmap* bmp = &ft.face_->glyph->bitmap;

    int bmp_w = bmp->width;
    int bmp_h = bmp->rows;
    
		if (pen_x + bmp_w >= metrics_.texture_width_)
    {
			pen_x = 0;
			pen_y += ((ft.face_->size->metrics.height >> 6) + 1);
		}

		for (int row = 0; row < bmp_h; ++row)
    {
			for (int col = 0; col < bmp_w; ++col)
      {
				int x = pen_x + col;
				int y = pen_y + row;
				atlas_data_[y * metrics_.texture_width_ + x] = bmp->buffer[row * bmp->pitch + col];
			}
		}

    const int base_line_to_bottom = ft.face_->glyph->bitmap_top - bmp_h;
    const int base_line_to_top = ft.face_->glyph->bitmap_top;

    Vec4i coords;
		coords.x0 = 0;
		coords.y0 = base_line_to_bottom;
		coords.x1 = bmp_w;
		coords.y1 = base_line_to_top;

    Vec4f uv;
    uv.u0 = pen_x * recip_width;
    uv.v0 = pen_y * recip_height;
    uv.u1 = (pen_x + bmp_w) * recip_width;
    uv.v1 = (pen_y + bmp_h) * recip_height;
	
    float advance = float(ft.face_->glyph->advance.x >> 6);

    characters_[static_cast<int>(c)] = Character { coords, uv, advance };

		pen_x += bmp_w + 1;
  };
}

void gdm::Font::Dump(const std::string& dump_name)
{
  int w = metrics_.texture_width_;
  int h = metrics_.texture_height_;

  std::vector<unsigned char> png_data(metrics_.texture_width_ * metrics_.texture_height_ * 4, 0); 

	for (int i = 0; i < (w * h); ++i)
  {
		png_data[i * 4 + 0] |= atlas_data_[i];
		png_data[i * 4 + 1] |= atlas_data_[i];
		png_data[i * 4 + 2] |= atlas_data_[i];
		png_data[i * 4 + 3] = 255;
	}

	stbi_write_png(dump_name.c_str(), w, h, 4, png_data.data(), w * 4);
}
