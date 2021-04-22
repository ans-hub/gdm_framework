// *************************************************************
// File:    texture_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TEX_FACT_H
#define AH_GDM_TEX_FACT_H

#include <vector>

#include "memory/defines.h"

#include "system/assert_utils.h"

#include "math/vector3.h"
#include "math/vector4.h"

#include "render/api.h"
#include "render/defines.h"
#include "render/texture.h"

#include "data/abstract_image.h"

#include "data_factory.h"
#include "image_factory.h"

namespace gdm {

using TextureHandle = Handle;

constexpr static gfx::EFormatType V_DEFAULT_TEXTURE_FMT = gfx::EFormatType::UNORM4;

struct AbstractTexture
{ 
  AbstractTexture(ImageHandle handle, gfx::EFormatType format);

  void SetTextureImpl(gfx::Texture* texture_impl) { ASSERT(!impl_); impl_ = texture_impl; }
  auto GetTextureImpl() const -> gfx::Texture& { return *impl_; }
  auto HasTextureImpl() const -> bool { return impl_; }

public:
  operator const gfx::Texture*() const { return impl_; }
  operator const gfx::Texture&() const { return *impl_; }
  operator gfx::Texture*() { return impl_; }
  operator gfx::Texture&() { return *impl_; }

  auto GetImageHandle() const -> ImageHandle { return image_; }
  auto GetFormat() const -> gfx::EFormatType { return format_; }

private:
  ImageHandle image_;
  gfx::EFormatType format_;
  gfx::Texture* impl_;

}; // struct AbstractTexture

struct TextureFactory : public DataFactory<AbstractTexture*>
{
  static auto Load(ImageHandle handle, gfx::EFormatType format) -> TextureHandle;
  static auto Load(const char* fpath, gfx::EFormatType format) -> TextureHandle;
  static auto Create(const char* fpath, gfx::EFormatType format, const Vec3u& whd, const Vec3f& rgb) -> TextureHandle;
  static auto Create(const char* name, gfx::EFormatType format, const AbstractImage::StorageType& raw, const Vec3u& whd) -> TextureHandle;
  static void Release(TextureHandle handle);

}; // struct TextureFactory

} // namespace gdm

#endif // AH_GDM_TEX_FACT_H
