// *************************************************************
// File:    dx_renderer.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_renderer.h"

#include <system/bits_utils.h>
#include <system/string_utils.h>
#include <system/diff_utils.h>

#include "dx11/dx_vertex_shader.h"
#include "dx11/dx_pixel_shader.h"
#include "dx11/dx_utils.h"
#include "dx11/dx_texture.h"

#include "rasterizer_desc.h"
#include "vertex_shader.h"
#include "pixel_shader.h"

gdm::DxRenderer::DxRenderer(HWND window_hndl, gfx::DeviceProps flags)
  : window_hndl_{window_hndl}
  , v_sync_{}
  , clear_color_{0.05f, 0.05f, 0.05f, 1.f}
  , device_{nullptr}
  , device_ctx_{nullptr}
  , swap_chain_{nullptr}
  , debug_marks_{nullptr}
  , depth_stencil_view_{nullptr}
  , depth_stencil_buffer_{nullptr}
  , depth_stencil_state_{nullptr}
  , rasterizer_state_{nullptr}
  , rasterizer_states_{}
  , blend_state_enabled_{nullptr}
  , blend_state_disabled_{nullptr}
  , viewport_{0}
  , vertex_stride_{0}
  , vertex_shader_ready_{false}
  , pixel_shader_ready_{false}
  , render_target_view_{nullptr}
  , render_target_views_{}
  , indicies_count_{0}
  , vertices_count_{0}
  , vs_shader_resources_{}
  , ps_shader_resources_{}
  , primitive_topology_{static_cast<D3D_PRIMITIVE_TOPOLOGY>(gfx::TRIANGLE_LIST)}
{
  ASSERTF(window_hndl_ != 0, "DxRenderer ctor: window handle invalid - %d", GetLastError());

  RECT client_rect;
  GetClientRect(window_hndl_, &client_rect);

  uint scr_w = client_rect.right - client_rect.left;
  uint scr_h = client_rect.bottom - client_rect.top;
  ASSERTF(scr_w != 0, "Screen width is zero");
  ASSERTF(scr_h != 0, "Screen height is zero");

  DXGI_SWAP_CHAIN_DESC scd;
  ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

  scd.BufferCount = 1;
  scd.BufferDesc.Width = scr_w;
  scd.BufferDesc.Height = scr_h;
  scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  scd.BufferDesc.RefreshRate = d3d_utils::QueryRefreshRate(scr_w, scr_h, v_sync_);
  scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  scd.OutputWindow = window_hndl_;
  scd.SampleDesc.Count = 1;
  scd.SampleDesc.Quality = 0;
  scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
  scd.Windowed = !d3d_utils::IsWindowFullscreen(window_hndl_);

  UINT create_device_flags = 0;

  if (bits::HasFlag(flags, gfx::DEBUG_DEVICE))
    create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;

  D3D_FEATURE_LEVEL feature_levels[] =
  {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
      D3D_FEATURE_LEVEL_9_3,
      D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1
  };

  D3D_FEATURE_LEVEL feature_level;

  HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                             nullptr, create_device_flags, feature_levels, _countof(feature_levels),
                                             D3D11_SDK_VERSION, &scd, &swap_chain_, &device_, &feature_level,
                                             &device_ctx_);
  if (hr != S_OK)
  {
    LOGF("Couldn't create dx 11.1 ctx with reason - %s", std::system_category().message(hr).c_str());
    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
                                       nullptr, create_device_flags, &feature_levels[1], _countof(feature_levels) - 1,
                                       D3D11_SDK_VERSION, &scd, &swap_chain_, &device_, &feature_level,
                                       &device_ctx_);
  }
  ASSERTF(hr == S_OK, "Couldn't create device and swap chain - %d", hr);

  hr = device_ctx_->QueryInterface(__uuidof(debug_marks_), reinterpret_cast<void**>(&debug_marks_));
  ASSERTF(hr == S_OK, "Couldn't init user defined annotation interface - %d", hr);

  ID3D11Texture2D *back_buffer;
  hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&back_buffer);
  ASSERTF(hr == S_OK, "Couldn't init back buffer of swap chain - %d", hr);

  hr = device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_);
  ASSERTF(hr == S_OK, "Couldn't associate back buffer with render target - %d", hr);

  render_target_views_[0] = render_target_view_;

  d3d_utils::Release(back_buffer);

  D3D11_TEXTURE2D_DESC depth_stencil_buf_desc;
  ZeroMemory(&depth_stencil_buf_desc, sizeof(D3D11_TEXTURE2D_DESC));

  depth_stencil_buf_desc.ArraySize = 1;
  depth_stencil_buf_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_buf_desc.CPUAccessFlags = 0;
  depth_stencil_buf_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_buf_desc.Width = scr_w;
  depth_stencil_buf_desc.Height = scr_h;
  depth_stencil_buf_desc.MipLevels = 1;
  depth_stencil_buf_desc.SampleDesc.Count = 1;
  depth_stencil_buf_desc.SampleDesc.Quality = 0;
  depth_stencil_buf_desc.Usage = D3D11_USAGE_DEFAULT;

  hr = device_->CreateTexture2D(&depth_stencil_buf_desc, nullptr, &depth_stencil_buffer_);
  ASSERTF(hr == S_OK, "Couldn't create depth buffer - %d", hr);

  hr = device_->CreateDepthStencilView(depth_stencil_buffer_, nullptr, &depth_stencil_view_);
  ASSERTF(hr == S_OK, "Couldn't use depth buffer with depth/stencil view - %d", hr);

  D3D11_DEPTH_STENCIL_DESC depth_stencil_state_desc_;
  memset(&depth_stencil_state_desc_, 0, sizeof(D3D11_DEPTH_STENCIL_DESC));

  depth_stencil_state_desc_.DepthEnable = true;
  depth_stencil_state_desc_.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_state_desc_.DepthFunc = D3D11_COMPARISON_LESS;
  depth_stencil_state_desc_.StencilEnable = false;

  hr = device_->CreateDepthStencilState(&depth_stencil_state_desc_, &depth_stencil_state_);
  ASSERTF(hr == S_OK, "Couldn't setup depth/stencil state - %d", hr);

  D3D11_RASTERIZER_DESC rasterizer_desc;
  memset(&rasterizer_desc, 0, sizeof(D3D11_RASTERIZER_DESC));

  rasterizer_desc.AntialiasedLineEnable = false;
  rasterizer_desc.CullMode = D3D11_CULL_BACK;
  rasterizer_desc.DepthBias = 0;
  rasterizer_desc.DepthBiasClamp = 0.0f;
  rasterizer_desc.DepthClipEnable = true;
  rasterizer_desc.FillMode = D3D11_FILL_SOLID;
  rasterizer_desc.FrontCounterClockwise = false;
  rasterizer_desc.MultisampleEnable = false;
  rasterizer_desc.ScissorEnable = false;
  rasterizer_desc.SlopeScaledDepthBias = 0.0f;

  rasterizer_states_[0] = rasterizer_state_;

  hr = device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_);
  ASSERTF(hr == S_OK, "Couldn't setup rasterizer state - %d", hr);

  viewport_.Width = static_cast<float>(scr_w);
  viewport_.Height = static_cast<float>(scr_h);
  viewport_.TopLeftX = 0.f;
  viewport_.TopLeftY = 0.f;
  viewport_.MinDepth = 0.f;
  viewport_.MaxDepth = 1.f;

  D3D11_BLEND_DESC blend_desc;
  memset(&blend_desc, 0, sizeof(D3D11_BLEND_DESC));

  blend_desc.RenderTarget[0].BlendEnable = true;
  blend_desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  blend_desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  blend_desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  blend_desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  blend_desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  blend_desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  blend_desc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

  hr = device_->CreateBlendState(&blend_desc, &blend_state_enabled_);
  ASSERTF(hr == S_OK, "Couldn't create active blend buffer - %d", hr);

  blend_desc.RenderTarget[0].BlendEnable = false;

  hr = device_->CreateBlendState(&blend_desc, &blend_state_disabled_);
  ASSERTF(hr == S_OK, "Couldn't create active blend buffer - %d", hr);
}

gdm::DxRenderer::~DxRenderer()
{
  d3d_utils::Release(depth_stencil_view_);
  d3d_utils::Release(render_target_view_);
  d3d_utils::Release(depth_stencil_buffer_);
  d3d_utils::Release(depth_stencil_state_);
  for (auto state : rasterizer_states_)
  {
    d3d_utils::Release(state.second);
  }
  d3d_utils::Release(blend_state_enabled_);
  d3d_utils::Release(blend_state_disabled_);
  d3d_utils::Release(debug_marks_);
  d3d_utils::Release(swap_chain_);
  d3d_utils::Release(device_ctx_);
  d3d_utils::Release(device_);
}

void gdm::DxRenderer::UseVxShader(VertexShader& shader)
{
  dx::VertexShader& shader_impl = shader.GetImpl();
  ID3D11VertexShader* shader_ptr = static_cast<ID3D11VertexShader*>(shader_impl.ptr_);
  ID3D11InputLayout* layout_ptr = shader_impl.input_layout_;

  ASSERT(shader_ptr);
  ASSERT(layout_ptr);

  device_ctx_->VSSetShader(shader_ptr, nullptr, 0);
  device_ctx_->IASetInputLayout(layout_ptr);
  
  vertex_stride_ = shader_impl.vertex_size_;
  ASSERT(vertex_stride_ != 0);
  vertex_shader_ready_ = true;
}

void gdm::DxRenderer::UsePxShader(PixelShader& shader)
{
  dx::PixelShader& shader_impl = shader.GetImpl();
  ID3D11PixelShader* shader_ptr = static_cast<ID3D11PixelShader*>(shader_impl.ptr_);
  ID3D11SamplerState* sampler_ptr = shader_impl.sampler_state_;

  ASSERTF(shader_ptr, "Pixel shader is nullptr");
  ASSERTF(sampler_ptr, "Sampler state is nullptr");

  device_ctx_->PSSetShader(shader_ptr, nullptr, 0);
  device_ctx_->PSSetSamplers(0, 1, &sampler_ptr);

  pixel_shader_ready_ = true;
}

void gdm::DxRenderer::ReinitBuffers()
{
  RECT client_rect;
  GetClientRect(window_hndl_, &client_rect);

  uint scr_w = client_rect.right - client_rect.left;
  uint scr_h = client_rect.bottom - client_rect.top;

  if (scr_w <= 0)
    scr_w = 1;
  if (scr_h <= 0)
    scr_h = 1;

  ResetRenderTargets();

  d3d_utils::Release(render_target_view_);
  d3d_utils::Release(depth_stencil_view_);
  d3d_utils::Release(depth_stencil_buffer_);

  swap_chain_->ResizeBuffers(1, scr_w, scr_h, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

  ID3D11Texture2D *back_buffer_tex;
  HRESULT hr = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&back_buffer_tex);
  ASSERTF(hr == S_OK, "Couldn't init back buffer of swap chain - %d", hr);

  hr = device_->CreateRenderTargetView(back_buffer_tex, nullptr, &render_target_view_);
  ASSERTF(hr == S_OK, "Couldn't associate back buffer with render target - %d", hr);

  d3d_utils::Release(back_buffer_tex);

  D3D11_TEXTURE2D_DESC depth_stencil_buf_desc;
  ZeroMemory(&depth_stencil_buf_desc, sizeof(D3D11_TEXTURE2D_DESC));

  depth_stencil_buf_desc.ArraySize = 1;
  depth_stencil_buf_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_stencil_buf_desc.CPUAccessFlags = 0;
  depth_stencil_buf_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_buf_desc.Width = scr_w;
  depth_stencil_buf_desc.Height = scr_h;
  depth_stencil_buf_desc.MipLevels = 1;
  depth_stencil_buf_desc.SampleDesc.Count = 1;
  depth_stencil_buf_desc.SampleDesc.Quality = 0;
  depth_stencil_buf_desc.Usage = D3D11_USAGE_DEFAULT;

  hr = device_->CreateTexture2D(&depth_stencil_buf_desc, nullptr, &depth_stencil_buffer_);
  ASSERTF(hr == S_OK, "Couldn't create depth buffer - %d", hr);

  hr = device_->CreateDepthStencilView(depth_stencil_buffer_, nullptr, &depth_stencil_view_);
  ASSERTF(hr == S_OK, "Couldn't use depth buffer with depth/stencil view - %d", hr);

  viewport_.Width = static_cast<float>(scr_w);
  viewport_.Height = static_cast<float>(scr_h);
  viewport_.TopLeftX = 0.f;
  viewport_.TopLeftY = 0.f;
  viewport_.MinDepth = 0.f;
  viewport_.MaxDepth = 1.f;

  device_ctx_->RSSetViewports(1, &viewport_);
}

void gdm::DxRenderer::SetClearColor(const Vec4f& rgba)
{
  clear_color_ = rgba;
}

void gdm::DxRenderer::SetClearColor(const Vec3f& rgb)
{
  clear_color_ = Vec4f(rgb, 1.f);
}

bool gdm::DxRenderer::AddRenderTarget(DxTexture& tex, uint uid)
{
  auto found = render_target_views_.find(uid);
  if (found == render_target_views_.end() || found->second == nullptr)
  {
    render_target_views_[uid] = tex.GetRenderTargetView();
    ASSERT(render_target_views_[uid]);
    return true;
  }
  return false;
}

bool gdm::DxRenderer::RemoveRenderTarget(uint uid)
{
  auto found = render_target_views_.find(uid);
  if (found != render_target_views_.end() && found->second != nullptr)
  {
    found->second = nullptr;
    PrepareRenderTargets();
    return true;
  }
  return false;
}

void gdm::DxRenderer::ClearRenderTarget(uint uid, const Vec4f& color)
{
  auto found = render_target_views_.find(uid);
  if (found != render_target_views_.end() && found->second != nullptr)
    device_ctx_->ClearRenderTargetView(found->second, &color.data[0]);
}

void gdm::DxRenderer::SetPixelShaderResource(ID3D11ShaderResourceView* res, std::size_t num)
{
  if (ps_shader_resources_.size() <= num)
    ps_shader_resources_.resize(num, nullptr);
  ps_shader_resources_[num] = res;
}

void gdm::DxRenderer::PrepareRenderTargets(gfx::PassFlags flags /* = 0 */)
{
  ResetRenderTargets();
  std::vector<ID3D11RenderTargetView*> rtv{};
  for (auto& rtv_pair : render_target_views_)
  {
    ID3D11RenderTargetView* rtv_curr = rtv_pair.second;
    if (rtv_curr == nullptr)
      continue;
    rtv.push_back(rtv_curr);

    if (!bits::HasFlag(flags, gfx::DONT_CLEAR_RTV))
      device_ctx_->ClearRenderTargetView(rtv_curr, &clear_color_.r);
  }

  if (depth_stencil_view_ && !bits::HasFlag(flags, gfx::DONT_CLEAR_DSV))
    device_ctx_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

  device_ctx_->OMSetRenderTargets(static_cast<UINT>(rtv.size()), rtv.data(), depth_stencil_view_);
  device_ctx_->OMSetDepthStencilState(depth_stencil_state_, 1);
}

void gdm::DxRenderer::ResetRenderTargets(gfx::PassFlags flags /* = 0 */)
{
  ID3D11RenderTargetView* null_rtv[k_max_rtv_];
  for (uint i = 0; i < k_max_rtv_; ++i)
    null_rtv[i] = nullptr;
  device_ctx_->OMSetRenderTargets(k_max_rtv_, null_rtv, nullptr);
}

void gdm::DxRenderer::RemovePixelShaderResource(ID3D11ShaderResourceView *res)
{
   for (std::size_t i = 0; i < ps_shader_resources_.size(); ++i)
    if (ps_shader_resources_[i] == res)
    {
      ps_shader_resources_[i] = nullptr;
      ID3D11ShaderResourceView *null_srv = nullptr;
      device_ctx_->PSSetShaderResources(static_cast<UINT>(i), 1, &null_srv);
    }
}

void gdm::DxRenderer::ClearPixelShaderResource()
{
  for (std::size_t i = 0; i < ps_shader_resources_.size(); ++i)
  {
    ID3D11ShaderResourceView *null_srv = nullptr;
    device_ctx_->PSSetShaderResources(static_cast<UINT>(i), 1, &null_srv);
  }
  ps_shader_resources_.clear();
}

void gdm::DxRenderer::SetCustomViewport(const D3D11_VIEWPORT &viewport)
{
  device_ctx_->RSSetViewports(1, &viewport);
}

void gdm::DxRenderer::SetDefaultViewport()
{
  device_ctx_->RSSetViewports(1, &viewport_);
}

void gdm::DxRenderer::UseDepthStencilView(ID3D11DepthStencilView* dsv)
{
  backup_depth_stencil_view_ = depth_stencil_view_;
  depth_stencil_view_ = dsv;
}

void gdm::DxRenderer::AddRasterizerState(uint uid, const RasterizerDesc& desc)
{
  D3D11_RASTERIZER_DESC rasterizer_desc;
  memset(&rasterizer_desc, 0, sizeof(D3D11_RASTERIZER_DESC));

  rasterizer_desc.AntialiasedLineEnable = false;
  rasterizer_desc.CullMode = static_cast<D3D11_CULL_MODE>(desc.cull_);
  rasterizer_desc.DepthBias = 0;
  rasterizer_desc.DepthBiasClamp = 0.0f;
  rasterizer_desc.DepthClipEnable = true;
  rasterizer_desc.FillMode = static_cast<D3D11_FILL_MODE>(desc.fill_);
  rasterizer_desc.FrontCounterClockwise = false;
  rasterizer_desc.MultisampleEnable = false;
  rasterizer_desc.ScissorEnable = false;
  rasterizer_desc.SlopeScaledDepthBias = 0.0f;

  ID3D11RasterizerState* state = nullptr;
  HRESULT hr = device_->CreateRasterizerState(&rasterizer_desc, &state);
  ASSERTF(hr == S_OK, "Couldn't add rasterizer state %u - %d", uid, hr);
  ASSERTF(rasterizer_states_.find(uid) == rasterizer_states_.end(), "Rasterizer state %u already exists", uid);
  rasterizer_states_[uid] = state;
}

void gdm::DxRenderer::SetRenderPrimitivesType(gfx::PrimitiveType type)
{
  primitive_topology_ = static_cast<D3D_PRIMITIVE_TOPOLOGY>(type);
}

void gdm::DxRenderer::BeginFrame(gfx::PassFlags flags /* = 0 */)
{
  if (bits::HasFlag(flags, gfx::RS_STATE))
    device_ctx_->RSSetState(rasterizer_state_);
  if (bits::HasFlag(flags, gfx::RS_VIEWPORT))
    device_ctx_->RSSetViewports(1, &viewport_);
  if (bits::HasFlag(flags, gfx::CLEAR_SHADER_RES))
  {
    vs_shader_resources_.clear();
    ps_shader_resources_.clear();
  }
  if (bits::HasFlag(flags, gfx::NULL_VBUFF))
    vertex_buffer_ = nullptr;
  if (bits::HasFlag(flags, gfx::NULL_IBUFF))
    index_buffer_ = nullptr;
}

void gdm::DxRenderer::BeforeRenderPass(gfx::PassFlags flags /* = 0 */)
{
  if (bits::HasFlag(flags, gfx::NULL_IBUFF))
    index_buffer_ = nullptr;
}

void gdm::DxRenderer::RenderPass(gfx::PassFlags flags /* = 0 */)
{
  const UINT offset = 0;

  ASSERT(vertex_buffer_);
  ASSERT(vertex_stride_ != 0);
  device_ctx_->IASetVertexBuffers(0, 1, &vertex_buffer_, &vertex_stride_, &offset);

  if (index_buffer_)
    device_ctx_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

  if (!vs_shader_resources_.empty())
    device_ctx_->VSSetShaderResources(0, static_cast<UINT>(vs_shader_resources_.size()), vs_shader_resources_.data());
  if (!ps_shader_resources_.empty())
    device_ctx_->PSSetShaderResources(0, static_cast<UINT>(ps_shader_resources_.size()), ps_shader_resources_.data());

  ASSERTF(primitive_topology_ != 0, "Unexpected 0 state of primitive topology");
  device_ctx_->IASetPrimitiveTopology(primitive_topology_);

  if (index_buffer_ && indicies_count_ > 0)
    device_ctx_->DrawIndexed(static_cast<UINT>(indicies_count_ * 3), 0, 0);
  else
    device_ctx_->Draw(vertices_count_, 0);
}

void gdm::DxRenderer::AfterRenderPass(gfx::PassFlags flags /* = 0 */)
{
  UNUSED(flags);
  vs_shader_resources_.clear();
  ps_shader_resources_.clear();
}

void gdm::DxRenderer::CopyVerticesToGpu(const std::vector<float>& vertices)
{
  D3D11_MAPPED_SUBRESOURCE res;
  ZeroMemory(&res, sizeof(D3D11_MAPPED_SUBRESOURCE));
  HRESULT hr = device_ctx_->Map(vertex_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &res); // disable gpu access
  ASSERT(hr == S_OK);
  memcpy(res.pData, &vertices[0], vertices.size() * sizeof(float));
  device_ctx_->Unmap(vertex_buffer_, 0);  // enable gpu access
  ASSERT(hr == S_OK);
}

void gdm::DxRenderer::SetVertexBuffer(ID3D11Buffer* buffer, uint vertices_count /* = 0 */)
{
  vertex_buffer_ = buffer;
  vertices_count_ = vertices_count;
}

void gdm::DxRenderer::SetIndexBuffer(ID3D11Buffer* buffer, std::size_t indicies_count)
{
  index_buffer_ = buffer;
  indicies_count_ = indicies_count;
}

void gdm::DxRenderer::SetPixelShaderResource(ID3D11ShaderResourceView* res)
{
  ps_shader_resources_.push_back(res);
}

void gdm::DxRenderer::EndFrame(gfx::PassFlags flags /* = 0 */)
{
  UNUSED(flags);
  swap_chain_->Present(static_cast<int>(v_sync_), 0);
}

void gdm::DxRenderer::ReportLiveObjects()
{
#ifdef _DEBUG
  ID3D11Debug *dbg_device = nullptr;
  HRESULT hr = device_->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void **>(&dbg_device));
  ENSUREF(hr == S_OK, "Couldn't create dx11 debug interface");
  if (hr == S_OK)
  {
    hr = dbg_device->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    ENSUREF(hr == S_OK, "Couldn't report live dx11 device objects");
    d3d_utils::Release(dbg_device);
  }
#endif
}

void gdm::DxRenderer::AlphaBlendingEnabled(bool enabled)
{
  ASSERT(device_ctx_);
  float bf[4] = {0.f, 0.f, 0.f, 0.f};
  if (enabled)
    device_ctx_->OMSetBlendState(blend_state_enabled_, bf, 0xffffffff);
  else
    device_ctx_->OMSetBlendState(blend_state_disabled_, bf, 0xffffffff);
}

void gdm::DxRenderer::StartProfiling(const char* mark, const Vec4f& color)
{
  UNUSED(color);
  if (debug_marks_)
    debug_marks_->BeginEvent(CSTR(mark));
}

void gdm::DxRenderer::EndProfiling(const char* mark)
{
  UNUSED(mark);
  if (debug_marks_)
    debug_marks_->EndEvent();
}

void gdm::DxRenderer::AddProfilingMark(const char* mark, const Vec4f& color)
{
  UNUSED(color);
  if (debug_marks_)
    debug_marks_->SetMarker(CSTR(mark)); 
}
