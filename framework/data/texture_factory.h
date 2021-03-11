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

#include "data_factory.h"
#include "image_factory.h"

namespace gdm {

using TextureHandle = Handle;

struct AbstractTexture
{
  enum class EFormatType
  {
    F1,
    F2,
    F3,
    F4,
    F4HALF,
    SRGB4,
    UNORM4,
    D24_UNORM_S8_UINT,
    D32_SFLOAT_S8_UINT,
    R8_UNORM,
    D16_UNORM,
    FORMAT_TYPE_MAX
  };
  
  AbstractTexture(ImageHandle handle);

  template<class T>
  void SetApiImage(T* image) { image_impl_ = static_cast<void*>(image); }
  template<class T>
  auto GetApiImage() const -> T* { return static_cast<T*>(image_impl_); }

  template<class T>
  auto SetApiImageView(T* image_view) {  image_view_impl_ = static_cast<void*>(image_view); }
  template<class T>
  auto GetApiImageView() const -> T* { return static_cast<T*>(image_view_impl_); }

  ImageHandle image_;
  EFormatType format_;
  void* image_impl_ = nullptr;
  void* image_view_impl_ = nullptr;

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
