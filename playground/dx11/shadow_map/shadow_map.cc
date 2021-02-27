// *************************************************************
// File:    shadow_map.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GFX_DX_API
#define GFX_DX_API
#endif

#include <windows.h>

#include <system/assert_utils.h>
#include <system/diff_utils.h>
#include <system/timer.h>
#include <system/fps_counter.h>
#include <system/hash_utils.h>

#include <math/general.h>
#include <math/matrix.h>
#include <math/vector3.h>

#include <window/win/main_window.h>
#include <window/win/dx_input.h>

#include <render/dx11/dx_debug_context.h>
#include <render/dx11/dx_material.h>
#include <render/dx11/dx_object.h>
#include <render/dx11/dx_renderer.h>
#include <render/dx11/dx_texture.h>
#include <render/dx11/dx_uniform.h>
#include <render/dx11/dx_defines.h>
#include <render/dx11/dx_vertex_shader.h>
#include <render/dx11/dx_pixel_shader.h>

#include "render/input_layout.h"
#include <render/vertex_shader.h>
#include <render/pixel_shader.h>
#include <render/camera_eul.h>
#include <render/colors.h>
#include <render/desc/rasterizer_desc.h>
#include <render/desc/input_layout_desc.h>
#include <render/desc/sampler_desc.h>

#include "desc/vert_sh_buffers.h"
#include "desc/frag_sh_buffers.h"
#include "desc/texture_desc.h"

using namespace gdm;

#define PROFILE_START(mark) { d3d.StartProfiling(mark, Vec4f(0,0,0,1)); }
#define PROFILE_END(mark) { d3d.EndProfiling(mark); }
#define PROFILE_MARK(mark) { d3d.AddProfilingMark(mark, Vec4f(0,0,0,1)); }

struct ShadowMap
{
  ShadowMap(ID3D11Device* device, int width, int height)
    : viewport_{}
    , main(device, width, height, TextureDescMain{})
    , depth(device, width, height, TextureDescDepth{})
  {
    viewport_.Width = static_cast<float>(width);
    viewport_.Height = static_cast<float>(height);
    viewport_.TopLeftX = 0.f;
    viewport_.TopLeftY = 0.f;
    viewport_.MinDepth = 0.f;
    viewport_.MaxDepth = 1.f;
  }
  D3D11_VIEWPORT viewport_;
  DxTexture main;
  DxTexture depth;
};

void UpdateCamera(CameraEul& cam, DxInput& input, float dt)
{
  cam.Rotate(input.GetMouseY(), input.GetMouseX());

  dt += dt * static_cast<int>(input.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

  if (input.IsKeyboardBtnHold(DIK_W))
    cam.Move(cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_S))
    cam.Move(-cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_A))
    cam.Move(-cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_D))
    cam.Move(cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_R))
    cam.Move(cam.GetTm().GetCol(1), dt);
  if (input.IsKeyboardBtnHold(DIK_F))
    cam.Move(-cam.GetTm().GetCol(1), dt);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
  UNUSED(prevInstance);
  UNUSED(cmdLine);

  MainWindow win (800, 600, "DirectX scene", MainWindow::CENTERED);
  Renderer d3d (win.GetHandle(), gfx::DEBUG_DEVICE | gfx::PROFILE_MARKS);
  d3d.AddRasterizerState(GDM_HASH("rasterizer_main"), StdRasterizerDesc());
  d3d.AddRasterizerState(GDM_HASH("rasterizer_shadow_map"), ReverseRasterizerDesc());
  d3d.AddRasterizerState(GDM_HASH("rasterizer_wireframe"), WireRasterizerDesc());
  DxInput input (win.GetHandle(), hInstance);
  input.SetMouseSensitive(0.2f);

  VertexShader flat_vx {"playground/dx11/shadow_map/shaders/flat_vert.hlsl", StdInputLayout{}, d3d};
  DxUniform<FlatVx_PFCB> flat_vx_PFCB (d3d.GetDevice());
  DxUniform<FlatVx_POCB> flat_vx_POCB (d3d.GetDevice());
  PixelShader flat_px {"playground/dx11/shadow_map/shaders/flat_frag.hlsl", StdSamplerDesc{}, d3d};
  DxUniform<FlatPx_POCB> flat_px_POCB (d3d.GetDevice());
  VertexShader text_vx {"playground/dx11/shadow_map/shaders/depth_vert.hlsl", StdInputLayout{}, d3d};  // uses the same buffers as in flat
  PixelShader text_px {"playground/dx11/shadow_map/shaders/depth_frag.hlsl", StdSamplerDesc{}, d3d};
  VertexShader plane_vx {"playground/dx11/shadow_map/shaders/plane_2d_vert.hlsl", StdInputLayout{}, d3d};
  PixelShader plane_px {"playground/dx11/shadow_map/shaders/plane_2d_frag.hlsl", StdSamplerDesc{}, d3d};
  VertexShader dbg_vx {"playground/dx11/shadow_map/shaders/debug_draw_vert.hlsl", WireInputLayout{}, d3d};
  PixelShader dbg_px {"playground/dx11/shadow_map/shaders/debug_draw_frag.hlsl", StdSamplerDesc{}, d3d};

  CameraEul camera {75.f, win.GetAspectRatio(), 0.1f, 100.f};
  camera.SetPos(Vec3f(0.f, 0.2f, -5.f));
  camera.SetMoveSpeed(2.f);

  std::string models_path {"assets/models/"};
  std::string materials_path {"assets/materials/"};
  std::string textures_path {"assets/textures/"};

  DxObject cube(d3d, models_path + "cube_test.ply", materials_path, textures_path);
  cube.SetPos(Vec3f(0.f, 1.f, 0.f));
  DxObject cube2(d3d, models_path + "cube_test.ply", materials_path, textures_path);
  cube2.SetTm(matrix::MakeScale(0.5f));
  cube2.SetPos(Vec3f(-2.f, 0.5f, 2.5f));
  DxObject stone(d3d, models_path + "stone.ply", materials_path, textures_path);
  stone.SetPos(Vec3f(-10.f, -3.1f, -20.f));
  DxObject plane(d3d, models_path + "plane_xy.ply", materials_path, textures_path);
  plane.SetPos(Vec3f(-7.8f, 3.8f));
  DxObject ground(d3d, models_path + "plane_xz.ply", materials_path, textures_path);
  ground.SetName(GDM_HASH("ground"));
  ground.SetPos(Vec3f(0.f, 0.f));
  ground.SetTm(matrix::MakeScale(5.f));

  DxObject lamp(d3d, models_path + "sphere.ply", materials_path, textures_path);
  lamp.SetName(GDM_HASH("lamp"));
  lamp.SetTm(matrix::MakeScale(0.1f));
  lamp.SetPos(Vec3f(5.f, 4.f, 5.f));
  float lamp_omega {10.f};

  std::vector<DxObject*> objects {};
  objects.push_back(&ground);
  objects.push_back(&stone);
  objects.push_back(&cube);
  objects.push_back(&cube2);
  objects.push_back(&lamp);

  ShadowMap shadow_map(d3d.GetDevice(), 2048, 2048);  // less tex - more flickering (edges "shimmering")
  DebugContext debug_ctx (d3d.GetDevice());

  MSG msg {0};
  Timer timer {60};
  FpsCounter fps {};

  bool is_ortho = true;
  bool is_wireframe = false;

  do {
    if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      timer.Start();
      float dt = timer.GetLastDt();

      input.Capture();
      UpdateCamera(camera, input, dt);

      if (input.IsKeyboardBtnPressed(DIK_0))
        is_ortho ^= 1;
      if (input.IsKeyboardBtnPressed(DIK_9))
        is_wireframe ^= 1;

      if (win.IsResized())
      {
        d3d.ReinitBuffers();
        camera.SetAspectRatio(win.GetAspectRatio());
      }
      win.ProcessInput(input);

      Mat4f lamp_rot = matrix::MakeRotateY(lamp_omega * dt);
      lamp.SetPos(lamp_rot * lamp.GetPos());

      PROFILE_START("::Begin frame");
      
      d3d.BeginFrame(gfx::RS_STATE | gfx::RS_VIEWPORT | gfx::CLEAR_SHADER_RES | gfx::NULL_VBUFF | gfx::NULL_IBUFF);
      d3d.SetClearColor(color::White);

      PROFILE_END("::Begin frame");

      Mat4f lamp_view (1.f);
      matrix::MakeLookAt(lamp_view, cube.GetPos(), lamp.GetPos(), lamp_view.GetCol(1));
      lamp.SetTm(lamp_view);
      Mat4f lamp_proj (1.f);
      if (is_ortho)
        lamp_proj = matrix::MakeOrthoLH(10, 10, 1, 20);//camera.z_near_, camera.z_far_);
      else
        lamp_proj = camera.GetProjectionMx();

      PROFILE_START("::Draw shadow map");

      d3d.UseVxShader(text_vx);
      d3d.UsePxShader(text_px);
      d3d.SetCustomViewport(shadow_map.viewport_);
      d3d.SetRenderPrimitivesType(gfx::TRIANGLE_LIST);
 
      flat_vx_PFCB->mx_vp = lamp_proj * matrix::InverseTransform(lamp.GetTm());
      flat_vx_PFCB.UploadToVS(0, d3d.GetDeviceCtx());

      d3d.SwitchOffBackBuffer();
      d3d.AddRenderTarget(shadow_map.main, GDM_HASH("shadow_map"));
      d3d.UseDepthStencilView(shadow_map.depth.GetDepthStencilView());
      d3d.PrepareRenderTargets();
      d3d.ClearRenderTarget(GDM_HASH("shadow_map"), color::White);
      d3d.ClearPixelShaderResource(); // remove it as it doing in BeforeRenderPAss

      for (auto& object : objects)
      {
        if (object->GetName() == GDM_HASH("lamp"))// || object->GetName() == GDM_HASH("ground"))
          continue;

        flat_vx_POCB->mx_model = object->GetTm();
        flat_vx_POCB.UploadToVS(1, d3d.GetDeviceCtx());

        for (auto& mesh : object->GetMeshes())
        {
          d3d.BeforeRenderPass();
          d3d.SetVertexBuffer(mesh.GetVertexBuffer());
          d3d.SetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndiciesCount());
          d3d.RenderPass();
          d3d.AfterRenderPass();  // rename to explicit ClearShaderResources()
        }
      }
      PROFILE_END("::Draw shadow map");

      PROFILE_START("::Back buffer draw");

      if (is_wireframe)
        d3d.UseRasterizerState(GDM_HASH("rasterizer_wireframe"));
      else
        d3d.UseRasterizerState(GDM_HASH("rasterizer_main"));

      d3d.BeginFrame(gfx::RS_STATE);
      d3d.UseVxShader(flat_vx);
      d3d.UsePxShader(flat_px);
      d3d.RemoveRenderTarget(GDM_HASH("shadow_map"));
      d3d.SwitchOnBackBuffer();
      d3d.SetDefaultDepthStencilView();
      d3d.PrepareRenderTargets(); // d3d.SetRenderTargets(d3d.GetValidRenderTargets());
      d3d.SetDefaultViewport();

      Mat4f cam_proj = camera.GetProjectionMx();  // todo: remake matrix mul to non member as we shouldn't change value of first matrix usually
      flat_vx_PFCB->mx_vp = cam_proj * camera.GetViewMx();  // we couldn't mul it directly since ^
      flat_vx_PFCB->mx_light_tm = lamp_proj * matrix::InverseTransform(lamp.GetTm());
      flat_vx_PFCB.UploadToVS(0, d3d.GetDeviceCtx()); // todo: d3d.UploadVsData(flat_vx_PFCB);

      for (auto& object : objects)
      {
        flat_vx_POCB->mx_model = object->GetTm();
        flat_vx_POCB.UploadToVS(1, d3d.GetDeviceCtx());

        for (auto& mesh : object->GetMeshes())
        {
          d3d.BeforeRenderPass();
          d3d.SetVertexBuffer(mesh.GetVertexBuffer());
          d3d.SetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndiciesCount());
          d3d.SetPixelShaderResource(mesh.GetDiffuseMap()); // todo: num
          d3d.SetPixelShaderResource(shadow_map.main.GetShaderResourceView());

          flat_px_POCB.SetData(mesh.GetMaterialProps());
          flat_px_POCB.UploadToPS(0, d3d.GetDeviceCtx());

          d3d.RenderPass();
          d3d.AfterRenderPass();
          d3d.ClearPixelShaderResource(); // remove it as it doing in BeforeRenderPAss
          d3d.RemovePixelShaderResource(shadow_map.main.GetShaderResourceView());
        }
      }

      PROFILE_END("::Back buffer draw");

      PROFILE_START("::Draw 2d tex");

      d3d.UseVxShader(plane_vx);
      d3d.UsePxShader(plane_px);

      float ortho_width  = 20.f; // wtf ??
      float ortho_height = ortho_width / win.GetAspectRatio();
      Mat4f ortho = matrix::MakeOrthoLH(ortho_width, ortho_height, -1.f, 1.f);  // for 2d texture drawing

      flat_vx_PFCB->mx_vp = ortho;
      flat_vx_PFCB.UploadToVS(0, d3d.GetDeviceCtx());
      flat_vx_POCB->mx_model = plane.GetTm();
      flat_vx_POCB.UploadToVS(1, d3d.GetDeviceCtx());
  
      for (auto& mesh : plane.GetMeshes())
      {
        d3d.BeforeRenderPass();
        d3d.SetVertexBuffer(mesh.GetVertexBuffer());
        d3d.SetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndiciesCount());
        d3d.SetPixelShaderResource(shadow_map.main.GetShaderResourceView());
        d3d.RenderPass();
        d3d.RemovePixelShaderResource(shadow_map.main.GetShaderResourceView());
        d3d.AfterRenderPass();
      }

      PROFILE_END("::Draw 2d tex");

      PROFILE_START("::Debug draw");

      debug_ctx.Clear();
      render_dbg::AddLinesFromTm(debug_ctx, cube.GetTm(), 1.f);
      render_dbg::AddLinesFromTm(debug_ctx, lamp.GetTm(), 1.f);
      render_dbg::AddLinesFromTm(debug_ctx, stone.GetTm(), 1.f);

      d3d.UseVxShader(dbg_vx);
      d3d.UsePxShader(dbg_px);
      flat_vx_PFCB->mx_vp = cam_proj * camera.GetViewMx();
      flat_vx_PFCB.UploadToVS(0, d3d.GetDeviceCtx());

      d3d.UseDepthStencilView(nullptr);
      d3d.PrepareRenderTargets(gfx::DONT_CLEAR_RTV | gfx::DONT_CLEAR_DSV);
      d3d.BeforeRenderPass(gfx::NULL_IBUFF); // todo: make device flags under separated namespace
      d3d.SetRenderPrimitivesType(gfx::LINE_LIST);
      d3d.SetVertexBuffer(debug_ctx.GetVertexBuffer(), debug_ctx.GetVerticesSize());
      d3d.CopyVerticesToGpu(debug_ctx.GetVertices());
      d3d.RenderPass();
      d3d.AfterRenderPass();
      d3d.SetDefaultDepthStencilView();

      PROFILE_END("::Debug draw");

      d3d.EndFrame();

      timer.End();
      timer.Wait();
    }

  } while (msg.message != WM_QUIT);

  d3d.ReportLiveObjects();

  return static_cast<int>(msg.wParam);
}
