// *************************************************************
// File:    image_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_IMG_FACT_H
#define AH_GDM_IMG_FACT_H

#include <vector>

#include "memory/defines.h"

#include "abstract_image.h"
#include "data_factory.h"

namespace gdm {

using ImageHandle = Handle;

struct ImageFactory : public DataFactory<AbstractImage*>
{
  static auto Create(const char* name, int w, int h, int d, float r, float g, float b) -> ImageHandle;
  static auto Load(const char* fpath) -> ImageHandle;
  static void Release(ImageHandle handle);

public:
  constexpr static const char* v_dummy_name = "dummy_image";
};

} // namespace gdm

#endif // AH_GDM_IMG_FACT_H
