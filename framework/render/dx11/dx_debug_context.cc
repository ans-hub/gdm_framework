// *************************************************************
// File:    debug_context.cc
// Descr:   debug draw sruff
// Author:  Novoselov Anton @ 2018-2019
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_debug_context.h"

#include "math/matrix.h"
#include "math/vector4.h"
#include "system/assert_utils.h"
#include "render/dx11/dx_utils.h"

gdm::DebugContext::DebugContext(ID3D11Device* device)
  : vertices_{}
  , vertices_count_{0}
  , vertex_buffer_{nullptr}
  , k_max_vertices_{1024}
{
  CreateVertexBuffer(device);
}

void gdm::DebugContext::CreateVertexBuffer(ID3D11Device* device)
{
  vertices_.resize(k_max_vertices_);

  D3D11_BUFFER_DESC vx_buffer_desc;
  ZeroMemory(&vx_buffer_desc, sizeof(D3D11_BUFFER_DESC));

  vx_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vx_buffer_desc.ByteWidth = static_cast<UINT>(vertices_.size() * sizeof(float));
  vx_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  vx_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;

  D3D11_SUBRESOURCE_DATA resource_data;
  ZeroMemory(&resource_data, sizeof(D3D11_SUBRESOURCE_DATA));
  resource_data.pSysMem = vertices_.data();

  HRESULT hr = device->CreateBuffer(&vx_buffer_desc, &resource_data, &vertex_buffer_);
  ASSERTF(hr == S_OK, "Can't create vertex buffer - %d", hr);
}

gdm::DebugContext::~DebugContext()
{
  d3d_utils::Release(vertex_buffer_);
}

void gdm::DebugContext::AddLine(const Vec3f& v0, const Vec3f& v1, const Vec4f& color)
{
  for (unsigned int i = 0; i < 3; ++i)
    vertices_.push_back(v0[i]);
  for (unsigned int i = 0; i < 4; ++i)
    vertices_.push_back(color[i]);
  ++vertices_count_;

  for (unsigned int i = 0; i < 3; ++i)
    vertices_.push_back(v1[i]);
  for (unsigned int i = 0; i < 4; ++i)
    vertices_.push_back(color[i]);
  ++vertices_count_;
}

void gdm::DebugContext::Clear(unsigned int type)
{
  vertices_.clear();
  vertices_count_ = 0;
}

void gdm::render_dbg::AddLinesFromTm(DebugContext& ctx, const Mat4f& tm, float length)
{
  ctx.AddLine(tm.GetCol(3), tm.GetCol(3) + tm.GetCol(0).Normalize() * length, Vec4f(1,0,0,1));
  ctx.AddLine(tm.GetCol(3), tm.GetCol(3) + tm.GetCol(1).Normalize() * length, Vec4f(0,1,0,1));
  ctx.AddLine(tm.GetCol(3), tm.GetCol(3) + tm.GetCol(2).Normalize() * length, Vec4f(0,0,1,1));
}
