// *************************************************************
// File:    texture_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TEX_FACT_H
#define AH_GDM_TEX_FACT_H

#include <vector>

#include "memory/defines.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "render/defines.h"
#include "render/texture.h"

#include "data_factory.h"
#include "image_factory.h"

namespace gdm {

using TextureHandle = Handle;

struct AbstractTexture
{ 
  AbstractTexture(ImageHandle handle);

  void SetTextureImpl(gfx::Texture* texture_impl) { impl_ = texture_impl; }
  auto GetTextureImpl() const -> gfx::Texture& { return *impl_; }
  auto HasTextureImpl() const -> bool { return impl_; }

  operator gfx::Texture*() const { return impl_; }
  operator gfx::Texture*() { return impl_; }

  ImageHandle image_ = v_null_handle;
  gfx::EFormatType format_ = gfx::EFormatType::FORMAT_TYPE_MAX;
  gfx::Texture* impl_ = nullptr;

}; // struct AbstractTexture

struct TextureFactory : public DataFactory<AbstractTexture*>
{
  static auto Load(ImageHandle handle) -> TextureHandle;
  static auto Load(const char* fpath) -> TextureHandle;
  static auto Create(const char* fpath, const Vec3u& whd, const Vec3f& rgb) -> TextureHandle;
  static auto Create(const char* name, const AbstractImage::StorageType& raw, const Vec3u& whd) -> TextureHandle;
  static void Release(TextureHandle handle); // TODO: Release what? only tex? or with chidlren? ref_cnt needed
};

} // namespace gdm

#endif // AH_GDM_TEX_FACT_H
