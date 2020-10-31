// *************************************************************
// File:    mesh_utils.cc
// Author:  Novoselov Anton @ 2018-2019
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <vector>

#include <math/general.h>
#include <math/vector2.h>
#include <math/vector3.h>
#include <math/vector4.h>
#include <math/matrix.h>

#ifndef AH_GDM_MESH_UTILS_H
#define AH_GDM_MESH_UTILS_H

namespace gdm::mesh_utils
{
  std::vector<Vec3f> ComputeVxsNormals(
    const std::vector<Vec3f>& coords, const std::vector<Vec3u>& faces);

  std::vector<Mat4f> ComputeVxsTangentSpace(
    const std::vector<Vec3f>& vxs_coords, const std::vector<Vec2f>& tex_coords,
    const std::vector<Vec3f>& vxs_normals, const std::vector<Vec3u>& faces);

} // namespace gdm::mesh_utils

#endif // AH_GDM_MESH_UTILS_H