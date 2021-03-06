// *************************************************************
// File:    model_factory.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MOD_FACT_H
#define AH_GDM_MOD_FACT_H

#include <vector>

#include "memory/defines.h"
#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"
#include "math/matrix.h"

#include "data/abstract_image.h"

#include "data_factory.h"
#include "mesh_factory.h"
#include "material_factory.h"

namespace gdm {

using ModelHandle = Handle;

enum LightType : int
{
  DIR, POINT, SPOTLIGHT
};

struct AbstractModel
{
  std::vector<MaterialHandle> materials_;
  std::vector<MeshHandle> meshes_;

}; // struct AbstractModel

struct ModelInstance
{
  ModelHandle handle_;
  Mat4f tm_;
  Vec4f color_;

}; // struct ModelInstance

struct ModelLight
{
  ModelInstance instance_;
  bool enabled_;
};

struct ModelFactory : public DataFactory<AbstractModel*>
{
  static auto Load(const char* fpath) -> ModelHandle;
  static void Release(ModelHandle handle);

}; // struct ModelFactory

} // namespace gdm

#endif // AH_GDM_MOD_FACT_H
