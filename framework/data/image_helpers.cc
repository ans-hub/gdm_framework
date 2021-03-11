// *************************************************************
// File:    image_helpers.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "image_helpers.h"

#include <set>
#include <filesystem>

#include "memory/defines.h"
#include "system/string_utils.h"

// --helpers

gdm::AbstractImage* gdm::helpers::MakeImage(const AbstractImage::StorageType& raw, int width, int height, int depth)
{
  return GMNew AbstractImage(raw, width, height, depth);
}

gdm::AbstractImage* gdm::helpers::MakeImage(const char* fname, std::array<float,3> rgb)
{
  std::string ext = str::GetFileExtension(fname);
  if (ext == "bmp")
	  return GMNew BmpImage(fname);
  else if (ext == "tga")
	  return GMNew TgaImage(fname);
  else if (ext == "png")
	  return GMNew PngImage(fname);
  else
	  return GMNew AbstractImage(32, 32, 32, rgb[0], rgb[1], rgb[2]);
}

void gdm::helpers::ReleaseImage(AbstractImage* img)
{
  GMDelete(img);
}
