// *************************************************************
// File:    image_helpers.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_IMAGE_HELPERS_H
#define AH_GDM_IMAGE_HELPERS_H

#include <vector>
#include <array>
#include <type_traits>

#include "abstract_image.h"
#include "bmp_loader.h"
#include "tga_loader.h"
#include "png_loader.h"

namespace gdm::helpers {

  auto MakeImage(const AbstractImage::StorageType& raw, int width, int height, int depth) -> AbstractImage*;
  auto MakeImage(const char* fname, std::array<float,3> rgb = {0.f, 0.f, 0.f}) -> AbstractImage*;
  void ReleaseImage(AbstractImage* img);

} // namespace gdm::helpers

#endif // AH_GDM_IMAGE_HELPERS_H