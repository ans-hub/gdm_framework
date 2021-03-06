// *************************************************************
// File:    material_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MAT_FACT_H
#define AH_GDM_MAT_FACT_H

#include <vector>

#include "memory/defines.h"
#include "math/vector4.h"

#include "texture_factory.h"

namespace gdm {

struct ModelLoader;

using MaterialHandle = Handle;

struct AbstractMaterial
{
  struct Props
  {
    Vec4f emissive_ = Vec4f(0.f);
    Vec4f ambient_ = Vec4f(0.1f);
    Vec4f diffuse_ = Vec4f(1.f);
    Vec4f specular_ = Vec4f(1.f);
    float specular_power_ = 32.f;
    Vec3f padding_;
  
  }; // struct Props
  
  auto GetTextureHandles() const -> std::vector<TextureHandle> { return {spec_, norm_, diff_}; }

  int index_ = -1;
  TextureHandle spec_;
  TextureHandle norm_;
  TextureHandle diff_;
  Props props_;
};

struct MaterialFactory : public DataFactory<AbstractMaterial*>
{
  static auto Create(const char* mat_name, int mat_num, const ModelLoader& loader) -> MaterialHandle;
  static void Release(MaterialHandle handle);
  static bool ImplementationLoaded(MaterialHandle handle);

public:
  constexpr static const char* v_dummy_name = "dummy_material";
};

} // namespace gdm

#endif // AH_GDM_MAT_FACT_H
