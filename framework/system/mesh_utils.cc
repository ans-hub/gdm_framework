// *************************************************************
// File:    mesh_utils.cc
// Author:  Novoselov Anton @ 2018-2019
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "mesh_utils.h"

// Simplest algorithm. To dive into futher:
//  http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.99.2846&rep=rep1&type=pdf
//  http://help.autodesk.com/view/3DSMAX/2016/ENU/?guid=__files_GUID_0FCB4578_77F8_4F05_99CD_349E85F13639_htm

std::vector<gdm::Vec3f> gdm::mesh_utils::ComputeVxsNormals(
  const std::vector<Vec3f>& coords, const std::vector<Vec3u>& faces)
{
  const std::size_t k_vxs_cnt = coords.size();
  std::vector<Vec3f> vxs_normals(k_vxs_cnt);

  for (const auto& face : faces)
  {
    std::size_t idx0 = face.x;
    std::size_t idx1 = face.y;
    std::size_t idx2 = face.z;
    assert(idx0 < k_vxs_cnt && idx1 < k_vxs_cnt && idx2 < k_vxs_cnt);
    Vec3f v0 = coords[idx0];
    Vec3f v1 = coords[idx1];
    Vec3f v2 = coords[idx2];
    Vec3f norm_wt = vec3::CrossProduct(v1-v0, v2-v0);
    vxs_normals[idx0] += norm_wt;
    vxs_normals[idx1] += norm_wt;
    vxs_normals[idx2] += norm_wt;
  }
  for (std::size_t i = 0; i < vxs_normals.size(); ++i)
    vxs_normals[i] = vec3::Normalize(vxs_normals[i]);
  return vxs_normals;
}

// Computes tangent space (TBN) matrix for vertices. Computes face-based TBN, then by
// averaging make it vertex-based TBN
// Also additional info:
//  http://www.thetenthplanet.de/archives/1180
//  https://fenix.tecnico.ulisboa.pt/downloadFile/1126518382171742/Bump%20Mapping.pdf
//  http://www.terathon.com/code/tangent.html

std::vector<gdm::Mat4f> gdm::mesh_utils::ComputeVxsTangentSpace(
  const std::vector<Vec3f>& vxs_coords, const std::vector<Vec2f>& tex_coords,
  const std::vector<Vec3f>& vxs_normals, const std::vector<Vec3u>& faces)
{
  assert(tex_coords.size() == vxs_coords.size());
  
  const std::size_t k_vxs_cnt = vxs_coords.size();
  std::vector<Mat4f> tbn_matrices(k_vxs_cnt);

  [[maybe_unused]] int zero_dividers = 0;

  for (const auto& face : faces)
  {
    std::size_t idx[3] = {face.x, face.y, face.z};
    assert(idx[0] < k_vxs_cnt && idx[1] < k_vxs_cnt && idx[2] < k_vxs_cnt);

    Vec3f edge_0 = vxs_coords[idx[1]] - vxs_coords[idx[0]];
    Vec3f edge_1 = vxs_coords[idx[2]] - vxs_coords[idx[0]];
    Vec2f dt_uv_0 = tex_coords[idx[1]] - tex_coords[idx[0]];
    Vec2f dt_uv_1 = tex_coords[idx[2]] - tex_coords[idx[0]];

    float divider = (dt_uv_0.x * dt_uv_1.y - dt_uv_1.x * dt_uv_0.y);
    
    Mat4f tbn (1.f);

    if (math::FNotZero(divider))
    {
      float f = 1.0f / divider;
      Vec3f tg = (edge_0 * dt_uv_1.y - edge_1 * dt_uv_0.y) * f;
      Vec3f bt = (edge_1 * dt_uv_0.x - edge_0 * dt_uv_1.x) * f;    
      tbn.SetCol(0, tg);;
      tbn.SetCol(1, bt);
    }
    else
      ++zero_dividers;

    tbn_matrices[idx[0]] += tbn;
    tbn_matrices[idx[1]] += tbn;
    tbn_matrices[idx[2]] += tbn;
  }

  assert(tbn_matrices.size() == vxs_normals.size());
  for (std::size_t i = 0; i < tbn_matrices.size(); ++i)
  {
    tbn_matrices[i].SetCol(0, vec3::Normalize(tbn_matrices[i].GetCol(0)));
    tbn_matrices[i].SetCol(1, vec3::Normalize(tbn_matrices[i].GetCol(1)));
    tbn_matrices[i].SetCol(2, vxs_normals[i]);
    // matrix::Orthonormalize(tbn_matrices[i]); // todo: why not
  }

  // Additional step - ortogonalization and handedness

  for (std::size_t i = 0; i < tbn_matrices.size(); ++i)
  {
    Vec3f tg = tbn_matrices[i].GetCol(0);
    Vec3f bt = tbn_matrices[i].GetCol(2);
    Vec3f nm = tbn_matrices[i].GetCol(1);
  
    tg = vec3::Normalize(tg - nm * vec3::DotProduct(nm, tg));
    if (vec3::DotProduct(vec3::CrossProduct(nm, tg), bt) < 0.0f)
      tg = tg * -1.0f;
    tbn_matrices[i].SetCol(0, tg);
  }
  return tbn_matrices;
}
