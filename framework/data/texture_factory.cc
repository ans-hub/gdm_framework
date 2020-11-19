// *************************************************************
// File:    texture_factory.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "texture_factory.h"

#include <filesystem>
#include <vector>

#include <system/hash_utils.h>
#include <system/assert_utils.h>
#include <system/array_utils.h>
#include <system/mesh_utils.h>

#include "helpers.h"
#include "model_loader.h"
#include "abstract_image.h"

#include "image_factory.h"

// --public AbstractTexture

gdm::AbstractTexture::AbstractTexture(ImageHandle handle)
  : image_{handle}
{ }
 
// --public TextureFactory

gdm::TextureHandle gdm::TextureFactory::Load(ImageHandle img_handle)
{
  ASSERTF(ImageFactory::Has(img_handle), "Image doesn't exists while creating texture");
  ASSERTF(!Has(img_handle), "Trying to load alreade loaded texture");

  AbstractImage* img = ImageFactory::Get(img_handle);
  AbstractTexture* tex = GMNew AbstractTexture(img_handle);
  resources_[img_handle] = tex;
  return img_handle;
}

gdm::TextureHandle gdm::TextureFactory::Load(const char* fpath)
{
  ASSERT(*fpath != '\000');
  Handle img_handle = ImageFactory::Has(fpath) ? ImageFactory::GetHandle(fpath) : ImageFactory::Load(fpath);
  Handle tex_handle = Load(img_handle);
  return tex_handle;
}

gdm::TextureHandle gdm::TextureFactory::Create(const char* fpath, const Vec3u& whd, const Vec3f& rgb)
{
  ASSERT(*fpath != '\000');
  Handle img_handle;
  if (ImageFactory::Has(fpath))
    img_handle = ImageFactory::GetHandle(fpath);
  else
    img_handle = ImageFactory::Create(fpath, whd.w, whd.h, whd.d, rgb.r, rgb.g, rgb.b);
  Handle tex_handle = Load(img_handle);
  return tex_handle;
}

void gdm::TextureFactory::Release(TextureHandle handle)
{
  ASSERTF(Has(handle), "Trying to release absent texture");

  AbstractTexture* tex = Get(handle);
  GMDelete(tex);
  resources_.erase(handle);
  ImageFactory::Release(handle);
}
