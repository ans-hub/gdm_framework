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
  void* image_impl_ = nullptr;
  void* image_view_impl_ = nullptr;

}; // struct AbstractTexture

struct TextureFactory : public DataFactory<AbstractTexture*>
{
  static auto Load(ImageHandle handle) -> TextureHandle;
  static auto Load(const char* fpath) -> TextureHandle;
  static auto Create(const char* fpath, const Vec3u& whd, const Vec3f& rgb) -> TextureHandle;
  static void Release(TextureHandle handle); // TODO: Release what? only tex? or with chidlren? ref_cnt needed
};

} // namespace gdm

#endif // AH_GDM_TEX_FACT_H
