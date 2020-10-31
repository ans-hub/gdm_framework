// *************************************************************
// File:    image_factory.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "image_factory.h"

#include <filesystem>
#include <vector>

#include <system/hash_utils.h>
#include <system/assert_utils.h>
#include <system/array_utils.h>
#include <system/mesh_utils.h>

#include "helpers.h"
#include "abstract_image.h"

// --public

gdm::ImageHandle gdm::ImageFactory::Load(const char* fpath)
{
  ASSERT(*fpath != '\000');  
  Handle handle = helpers::GenerateHandle(fpath);
  ASSERTF(!Has(handle), "Trying to load already loaded image");
  std::string full_path = std::string(resources_path_) + std::string(fpath);
  AbstractImage* img = helpers::MakeImage(full_path.c_str());
  resources_[handle] = img;
  return handle;
}

void gdm::ImageFactory::Release(ImageHandle handle)
{
  ASSERTF(Has(handle), "Trying to release absent image");

  AbstractImage* img = Get(handle);
  helpers::ReleaseImage(img);
  resources_.erase(handle);
}
