// *************************************************************
// File:    texture_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TEX_FACT_H
#define AH_GDM_TEX_FACT_H

#include <vector>

#include "memory/defines.h"
#include "math/vector4.h"

#include "data_factory.h"
#include "image_factory.h"

namespace gdm {

using TextureHandle = Handle;

struct AbstractTexture
{
  ImageHandle image_; 
  void* impl_;

  AbstractTexture(ImageHandle handle)
    : image_{handle}
    , impl_{nullptr}
  { }
  ~AbstractTexture()
  {
    // delete(texture_impl_); // uncomment when include all
  }
  
}; // struct AbstractTexture

struct TextureFactory : public DataFactory<AbstractTexture*>
{
  static auto Load(ImageHandle handle) -> TextureHandle;  // TODO: pass GpuDevice()???
  static auto Load(const char* fpath) -> TextureHandle;   //       same?
  static void Release(TextureHandle handle); // TODO: Release what? only tex? or with chidlren? ref_cnt needed
};

} // namespace gdm

#endif // AH_GDM_TEX_FACT_H
