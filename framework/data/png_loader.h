// *************************************************************
// File:    png_loader.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <iostream>
#include <fstream>
#include <vector>

#include "abstract_image.h"

#ifndef AH_GDM_PNG_LOADER_H
#define AH_GDM_PNG_LOADER_H

namespace gdm {

struct PngImage : public AbstractImage
{
  PngImage(const char* fpath);

}; // struct PngImage

} // namespace gdm

#endif // AH_GDM_PNG_LOADER_H