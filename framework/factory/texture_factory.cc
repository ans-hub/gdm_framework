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

#include "data/image_helpers.h"
#include "data/model_loader.h"
#include "data/abstract_image.h"

#include "image_factory.h"

// --public

gdm::AbstractTexture::AbstractTexture(ImageHandle handle, gfx::EFormatType format)
  : image_{handle}
  , format_{format}
  , impl_{nullptr}
{ }

gdm::TextureHandle gdm::TextureFactory::Load(ImageHandle img_handle, gfx::EFormatType format)
{
  ASSERTF(ImageFactory::Has(img_handle), "Image doesn't exists while creating texture");
  ASSERTF(!Has(img_handle), "Trying to load alreade loaded texture");

  AbstractImage* img = ImageFactory::Get(img_handle);
  AbstractTexture* tex = GMNew AbstractTexture(img_handle, format);
  resources_[img_handle] = tex;
  return img_handle;
}

gdm::TextureHandle gdm::TextureFactory::Load(const char* fpath, gfx::EFormatType format)
{
  ASSERT(*fpath != '\000');
  Handle img_handle = ImageFactory::Has(fpath) ? ImageFactory::GetHandle(fpath) : ImageFactory::Load(fpath);
  Handle tex_handle = Load(img_handle, format);
  return tex_handle;
}

gdm::TextureHandle gdm::TextureFactory::Create(const char* fpath, gfx::EFormatType format, const Vec3u& whd, const Vec3f& rgb)
{
  ASSERT(*fpath != '\000');
  Handle img_handle;
  if (ImageFactory::Has(fpath))
    img_handle = ImageFactory::GetHandle(fpath);
  else
    img_handle = ImageFactory::Create(fpath, whd.w, whd.h, whd.d, rgb.r, rgb.g, rgb.b);
  Handle tex_handle = Load(img_handle, format);
  return tex_handle;
}

gdm::TextureHandle gdm::TextureFactory::Create(const char* name, gfx::EFormatType format, const AbstractImage::StorageType& raw, const Vec3u& whd)
{
  ImageHandle img_handle = ImageFactory::Create(name, raw, whd.w, whd.h, whd.d);
  Handle tex_handle = Load(img_handle, format);
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
