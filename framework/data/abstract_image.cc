// *************************************************************
// File:    abstract_image.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <cassert>

#include "abstract_image.h"

// --public

gdm::AbstractImage::AbstractImage(int width, int height, int depth, float r, float g, float b, float a)
	: width_{width}
	, height_{height}
	, depth_{depth}
	, data_{}
{
  assert(depth == 24 || depth == 32);

  r = r < 0.f ? 0.f : (r > 1.f ? 1.f : r);
  g = g < 0.f ? 0.f : (g > 1.f ? 1.f : g);
  b = b < 0.f ? 0.f : (b > 1.f ? 1.f : b);
  a = a < 0.f ? 0.f : (a > 1.f ? 1.f : a);

  unsigned char color[4];
  color[0] = static_cast<unsigned char>(255 * r);
  color[1] = static_cast<unsigned char>(255 * g);
  color[2] = static_cast<unsigned char>(255 * b);
  color[3] = static_cast<unsigned char>(255 * a);

  int color_comps = depth_ / 8;
  int image_size = width_ * height_ * depth_ / 8;
  data_.resize(image_size);
  for (unsigned short i = 0; i < image_size; i += depth_ / 8)
    for (int k = 0; k < color_comps; ++k)
      data_[i+k] = color[k];
}

int gdm::AbstractImage::GetWidth() const
{
  return width_;
}

int gdm::AbstractImage::GetHeight() const
{
  return height_;
}

int gdm::AbstractImage::GetDepth() const
{
  return depth_;
}

auto gdm::AbstractImage::GetRaw() const -> const std::vector<unsigned char>&
{
  return data_;
}