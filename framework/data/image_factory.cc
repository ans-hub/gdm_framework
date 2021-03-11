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

#include "image_helpers.h"
#include "abstract_image.h"

// --public

auto gdm::ImageFactory::Create(const char* name, int w, int h, int d, float r, float g, float b) -> ImageHandle
{
  ASSERT(!Has(name));
	AbstractImage* img = GMNew AbstractImage(w, h, d, r, g, b);
  Handle handle = helpers::GenerateHandle(name);
  resources_[handle] = img;
  return handle;
}

auto gdm::ImageFactory::Create(const char* name, const AbstractImage::StorageType& raw, int w, int h, int d) -> ImageHandle
{
  ASSERT(!Has(name));
	AbstractImage* img = GMNew AbstractImage(raw, w, h, d);
  Handle handle = helpers::GenerateHandle(name);
  resources_[handle] = img;
  return handle;
}

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
