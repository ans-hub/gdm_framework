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

#include "abstract_image.h"
#include "data_factory.h"
#include "mesh_factory.h"
#include "material_factory.h"

namespace gdm {

using ModelHandle = Handle;

struct AbstractModel
{
  std::vector<MaterialHandle> materials_;
  std::vector<MeshHandle> meshes_;
  Mat4f tm_;

}; // struct AbstractModel

struct ModelFactory : public DataFactory<AbstractModel*>
{
  static auto Load(const char* fpath) -> ModelHandle;
  static void Release(ModelHandle handle);

}; // struct ModelFactory

} // namespace gdm

#endif // AH_GDM_MOD_FACT_H
