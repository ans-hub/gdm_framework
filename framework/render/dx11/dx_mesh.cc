// *************************************************************
// File:    dx_mesh.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_mesh.h"

gdm::DxMesh::DxMesh(ModelLoader& model, ID3D11Device* device, std::size_t mesh_num)
  : faces_{}
  , coords_{}
  , normals_{}
  , tangents_{}
  , texuv_{}
  , material_{nullptr}
  , vx_total_sz_{}
  , interleaving_vxs_buffer_ {}
  , interleaving_vxs_buffer_sz_{}
  , normalize_poses_{false}
  , vertex_buffer_{nullptr}
  , index_buffer_{nullptr}
{
  LoadData(model, mesh_num);

  AddToVxsBuffer(coords_);
  AddToVxsBuffer(texuv_);
  AddToVxsBuffer(normals_);
  AddToVxsBuffer(tangents_);

  CreateVertexBuffer(device);
  CreateIndexBuffer(device);
}

gdm::DxMesh::~DxMesh()
{
  d3d_utils::Release(vertex_buffer_);
  d3d_utils::Release(index_buffer_);
}

gdm::DxMesh::DxMesh(DxMesh&& mesh)
  : faces_{std::move(mesh.faces_)}
  , coords_{std::move(mesh.coords_)}
  , normals_{std::move(mesh.normals_)}
  , tangents_{std::move(mesh.tangents_)}
  , texuv_{std::move(mesh.texuv_)}
  , material_{mesh.material_}
  , vx_total_sz_{mesh.vx_total_sz_}
  , interleaving_vxs_buffer_ {std::move(mesh.interleaving_vxs_buffer_ )}
  , interleaving_vxs_buffer_sz_{mesh.interleaving_vxs_buffer_sz_}
  , vertex_buffer_{mesh.vertex_buffer_}
  , index_buffer_{mesh.index_buffer_}
{
  mesh.vertex_buffer_ = nullptr;
  mesh.index_buffer_ = nullptr;
}

gdm::DxMesh& gdm::DxMesh::operator=(DxMesh&& mesh)
{
  if (this != &mesh)
  {
    faces_ = std::move(mesh.faces_);
    coords_ = std::move(mesh.coords_);
    normals_ = std::move(mesh.normals_);
    tangents_ = std::move(mesh.tangents_);
    texuv_ = std::move(mesh.texuv_);
    material_ = mesh.material_;
    vx_total_sz_ = mesh.vx_total_sz_;
    interleaving_vxs_buffer_  = std::move(mesh.interleaving_vxs_buffer_ );
    interleaving_vxs_buffer_sz_ = mesh.interleaving_vxs_buffer_sz_;
    vertex_buffer_ = mesh.vertex_buffer_;
    index_buffer_ = mesh.index_buffer_;
    mesh.vertex_buffer_ = nullptr;
    mesh.index_buffer_ = nullptr;
  }
  return *this;
}

void gdm::DxMesh::SetMaterial(DxMaterial* material, ID3D11Device* device/*, std::size_t mesh_num*/)
{
  ASSERT(device);
  ASSERT(material);

  material_ = material;
}

void gdm::DxMesh::LoadData(ModelLoader& model, std::size_t mesh_num)
{
  faces_ = model.LoadIndicies<Vec3u>(mesh_num);
  assert(!faces_.empty());

  coords_ = model.LoadPositions<Vec3f>(mesh_num);
  if (normalize_poses_)
    arr_utils::Normalize(&coords_[0].x, coords_.size() * 3, -1.f, 1.f);
  assert(!coords_.empty());

  normals_ = model.LoadNormals<Vec3f>(mesh_num);
  if (normals_.empty())
    normals_ = mesh_utils::ComputeVxsNormals(coords_, faces_);
  assert(normals_.size() == coords_.size());

  texuv_ = model.LoadUV<Vec2f>(mesh_num);
  if (!texuv_.empty())
  {
    assert(coords_.size() == texuv_.size());
    std::vector<Mat4f> vxs_tbns = mesh_utils::ComputeVxsTangentSpace(coords_, texuv_, normals_, faces_);
    assert(vxs_tbns.size() == coords_.size());
    std::vector<Vec3f> tangents {};
    for (const auto& tbn : vxs_tbns)
      tangents_.push_back(vec3::Normalize(tbn.GetCol(0)));
    assert(tangents_.size() == coords_.size());
  }
  else // make fake uv coords and tangents for uniform shader access
  {
    texuv_.resize(coords_.size(), Vec2f{0.5f, 0.5f});
    tangents_.resize(coords_.size(), Vec3f{0.f, 1.f, 0.f});
  }
}

void gdm::DxMesh::CreateVertexBuffer(ID3D11Device* device)
{
  D3D11_BUFFER_DESC vx_buffer_desc;
  ZeroMemory(&vx_buffer_desc, sizeof(D3D11_BUFFER_DESC));

  vx_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vx_buffer_desc.ByteWidth = static_cast<UINT>(interleaving_vxs_buffer_ .size() * sizeof(float));
  vx_buffer_desc.CPUAccessFlags = 0;
  vx_buffer_desc.Usage = D3D11_USAGE_DEFAULT;

  D3D11_SUBRESOURCE_DATA resource_data;
  ZeroMemory(&resource_data, sizeof(D3D11_SUBRESOURCE_DATA));

  resource_data.pSysMem = &interleaving_vxs_buffer_ [0];

  HRESULT hr = device->CreateBuffer(&vx_buffer_desc, &resource_data, &vertex_buffer_);
  ASSERTF(hr == S_OK, "Can't create vertex buffer - %d", hr);
}

void gdm::DxMesh::CreateIndexBuffer(ID3D11Device* device)
{
  D3D11_BUFFER_DESC index_buffer_desc;
  ZeroMemory(&index_buffer_desc, sizeof(D3D11_BUFFER_DESC));

  index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  index_buffer_desc.ByteWidth = static_cast<UINT>(faces_.size() * 3 * sizeof(unsigned int));
  index_buffer_desc.CPUAccessFlags = 0;
  index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  
  D3D11_SUBRESOURCE_DATA resource_data;
  ZeroMemory(&resource_data, sizeof(D3D11_SUBRESOURCE_DATA));

  resource_data.pSysMem = &faces_[0];

  HRESULT hr = device->CreateBuffer(&index_buffer_desc, &resource_data, &index_buffer_);
  ASSERTF(hr == S_OK, "Can't create index buffer - %d", hr);
}
