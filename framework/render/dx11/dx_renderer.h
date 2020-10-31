// *************************************************************
// File:    dx_renderer.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_RENDERER_H
#define AH_DX_RENDERER_H

#include <map>
#include <vector>

#ifdef _WIN32
#include <d3d11.h>
#include <d3d11_1.h>
#endif

#include "render/defines.h"
#include "render/dx11/dx_defines.h"

#include "math/vector3.h" 
#include "math/vector4.h" 

namespace gdm {

struct DxTexture;
struct VxShader;
struct PxShader;
struct RasterizerDesc;
struct VertexShader;
struct PixelShader;

struct DxRenderer
{
  DxRenderer(HWND window_hndl, gfx::DeviceProps flags = 0);
  ~DxRenderer();

  auto GetDevice() -> ID3D11Device* { return device_; }
  auto GetCompiler() -> void* { return nullptr; }
  auto GetDeviceCtx() -> ID3D11DeviceContext* { return device_ctx_; }
  void UseVxShader(VertexShader& vx_shader);
  void UsePxShader(PixelShader& px_shader);

  bool AddRenderTarget(DxTexture& tex, uint uid);
  bool RemoveRenderTarget(uint uid);
  void ClearRenderTarget(uint uid, const Vec4f& color);
  void SwitchOnBackBuffer() { render_target_views_[0] = render_target_view_; }
  void SwitchOffBackBuffer() { render_target_views_[0] = nullptr; }
  void PrepareRenderTargets(gfx::PassFlags flags = 0);
  void ResetRenderTargets(gfx::PassFlags flags = 0);

  void SetCustomViewport(const D3D11_VIEWPORT& viewport);
  void SetDefaultViewport();

  void UseDepthStencilView(ID3D11DepthStencilView* dsv);
  void SetDefaultDepthStencilView() { depth_stencil_view_ = backup_depth_stencil_view_; }

  void AddRasterizerState(uint uid, const RasterizerDesc& desc);
  void UseRasterizerState(uint uid) { rasterizer_state_ = rasterizer_states_[uid]; }
  void UseDefaultRasterizerState() { rasterizer_state_ = rasterizer_states_[0]; }

  void SetRenderPrimitivesType(gfx::PrimitiveType type);

  void BeginFrame(gfx::PassFlags flags = 0);
  void EndFrame(gfx::PassFlags flags = 0);
  void BeforeRenderPass(gfx::PassFlags flags = 0);
  void RenderPass(gfx::PassFlags flags = 0);
  void AfterRenderPass(gfx::PassFlags flags = 0);
  void CopyVerticesToGpu(const std::vector<float>& vertices);
 
  void SetVertexBuffer(ID3D11Buffer* buffer, uint vertices_count = 0);
  void SetIndexBuffer(ID3D11Buffer* buffer, std::size_t indicies_count);
  void SetPixelShaderResource(ID3D11ShaderResourceView* res);
  void SetPixelShaderResource(ID3D11ShaderResourceView* res, std::size_t num);
  void RemovePixelShaderResource(ID3D11ShaderResourceView* res);
  void ClearPixelShaderResource();
  void SetVertexShaderResource(ID3D11ShaderResourceView* res) { vs_shader_resources_.push_back(res); }

  void ReinitBuffers();
  void SetClearColor(const Vec4f& rgba);
  void SetClearColor(const Vec3f& rgb);
  void AlphaBlendingEnabled(bool enabled);
  void ReportLiveObjects();
  void StartProfiling(const char* mark, const Vec4f& color);
  void EndProfiling(const char* mark);
  void AddProfilingMark(const char* mark, const Vec4f& color);

private:
  HWND window_hndl_;
  int v_sync_ = false;
  Vec4f clear_color_;

  ID3D11Device* device_;
  ID3D11DeviceContext* device_ctx_;
  IDXGISwapChain* swap_chain_;
  ID3DUserDefinedAnnotation* debug_marks_;

  D3D11_VIEWPORT viewport_;
  D3D_PRIMITIVE_TOPOLOGY primitive_topology_;

  static const uint k_max_rtv_ = 3;
  ID3D11RenderTargetView* render_target_view_;
  std::map<uint, ID3D11RenderTargetView*> render_target_views_;
  ID3D11DepthStencilView* depth_stencil_view_;
  ID3D11DepthStencilView* backup_depth_stencil_view_;

  ID3D11Texture2D* depth_stencil_buffer_;
  ID3D11Buffer* vertex_buffer_;
  ID3D11Buffer* index_buffer_;
  std::size_t indicies_count_;
  uint vertices_count_;
  uint vertex_stride_;
 
  ID3D11RasterizerState* rasterizer_state_;
  std::map<uint, ID3D11RasterizerState*> rasterizer_states_;
  ID3D11DepthStencilState* depth_stencil_state_;
  ID3D11BlendState* blend_state_enabled_;
  ID3D11BlendState* blend_state_disabled_;

  std::vector<ID3D11ShaderResourceView*> vs_shader_resources_;
  std::vector<ID3D11ShaderResourceView*> ps_shader_resources_;
  bool vertex_shader_ready_;
  bool pixel_shader_ready_;
  
}; // struct DxRenderer

} // namespace gdm

#endif // AH_DX_RENDERER_H
