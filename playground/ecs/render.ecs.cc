// *************************************************************
// File:    render.ecs.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "render.ecs.h"

#include <memory>

#include <render/dx11/dx_renderer.h>
#include <render/dx11/dx_shader.h>
#include <render/dx11/dx_uniform.h>
#include <render/dx11/dx_device_desc.h>
#include <render/dx11/dx_shader_desc.h>
#include <render/colors.h>

#include <math/matrix.h>
#include <math/vector2.h>
#include <math/vector3.h>
#include <math/vector4.h>

#include <window/win/main_window.h>

#include <system/hash_utils.h>

#include <ecs/manager.h>
#include <ecs/system.h>

#include <math/matrix.h>

#include "transform.ecs.h"

using namespace gdm;
using namespace ecs;

struct RasterizerMain : public RasterizerDesc
{
  RasterizerMain() : RasterizerDesc()
  {
    cull_ = ECullMode::BACK_FACE;
    fill_ = EFillMode::SOLID;
  }
};

struct VxFlat : public VxBase
{
  VxFlat() : VxBase
  ({
    VxElement("COORD", sizeof(Vec3f), VxElement::F3),
    VxElement("TEXUV" , sizeof(Vec2f), VxElement::F2),
    VxElement("NORMAL", sizeof(Vec3f), VxElement::F3),
    VxElement("TANGENT", sizeof(Vec3f), VxElement::F3)
  })
  { }
};

struct PxFlat : public PxBase
{
  PxFlat() : PxBase()
  {
    address_ = ETextureAddressMode::BORDER;
    border_color_ = { 1.f, 1.f, 1.f, 1.f };
    comparsion_ = ECompareFunc::ALWAYS;
  }
};

__declspec(align(16)) struct Vx_PFCB
{
  alignas(16) Mat4f u_view_proj;
};

__declspec(align(16)) struct Vx_POCB
{
  alignas(16) Mat4f u_model;
};

struct RenderPimpl
{
  VxShader<VxFlat> vx_shader;
  PxShader<PxFlat> px_shader;
  DxUniform<Vx_PFCB> vx_pfcb;
  DxUniform<Vx_POCB> vx_pocb;
  std::vector<DxObject*> renderables;

  RenderPimpl(DxRenderer& d3d)
    : vx_shader{L"shaders/vert.hlsl", d3d.GetDevice()}
    , px_shader{L"shaders/frag.hlsl", d3d.GetDevice()}
    , vx_pfcb{d3d.GetDevice()}
    , vx_pocb{d3d.GetDevice()}
    , renderables{}
  {
    d3d.AddRasterizerState(GDM_HASH("rasterizer_main"), RasterizerMain());
  }
};

Render::Render()
  : window{ecs::EntityManager::GetInstance().GetComponent<Window>()}
  , camera{ecs::EntityManager::GetInstance().GetComponent<Camera>()}
  , d3d{window->GetHandle(), gfx::DEBUG_DEVICE}
  , impl{std::make_unique<RenderPimpl>(d3d)}
{ }

Render::~Render()
{

}

Renderable::Renderable(const char* model_fpath)
  : object(ecs::EntityManager::GetInstance().GetComponent<Render>().d3d, model_fpath)
{ }

void render_begin_frame(Render& r)
{
  r.d3d.BeginFrame(gfx::RS_STATE | gfx::RS_VIEWPORT | gfx::CLEAR_SHADER_RES | gfx::NULL_VBUFF | gfx::NULL_IBUFF);
  r.d3d.SetClearColor(color::White);
}

void render_end_frame(Render& r)
{
  r.d3d.EndFrame();
}

void render_pass_main_prepare(Render& r)
{
  r.d3d.UseRasterizerState(GDM_HASH("rasterizer_main"));
  r.d3d.BeginFrame(gfx::RS_STATE);
  r.d3d.UseVxShader(r.impl->vx_shader);
  r.d3d.UsePxShader(r.impl->px_shader);
  r.d3d.SwitchOnBackBuffer();
  r.d3d.SetDefaultDepthStencilView();
  r.d3d.PrepareRenderTargets();
  r.d3d.SetDefaultViewport();
  r.impl->renderables.clear();
}

void render_pass_main_set_camera(Render& r, const Camera& camera)
{
  Mat4f cam_proj = camera->GetProjectionMx();
  r.impl->vx_pfcb->u_view_proj = cam_proj * camera->GetViewMx();
  r.impl->vx_pfcb.UploadToVS(0, r.d3d.GetDeviceCtx());
}

void render_pass_main_prepare_objects(Renderable& object, Transform& tm)
{
  Render& r = ecs::EntityManager::GetInstance().GetComponent<Render>();
  object->SetTm(*tm);
  r.impl->renderables.push_back(&*object);
}

void render_pass_main_sort_objects(Render& r, const Camera& cam)
{
  std::sort(r.impl->renderables.rbegin(), r.impl->renderables.rend(),
  [&](const DxObject* lhs, const DxObject* rhs){
    return vec3::SqLength(lhs->GetPos() - cam->GetPos()) < vec3::SqLength(rhs->GetPos() - cam->GetPos());
  });
}

void render_pass_main_draw_objects(Render& r)
{
  for (auto& object : r.impl->renderables)
  {
    r.impl->vx_pocb->u_model = object->GetTm();
    r.impl->vx_pocb.UploadToVS(1, r.d3d.GetDeviceCtx());

    for (auto& mesh : object->GetMeshes())
    {
      r.d3d.BeforeRenderPass();
      r.d3d.SetVertexBuffer(mesh.GetVertexBuffer());
      r.d3d.SetIndexBuffer(mesh.GetIndexBuffer(), mesh.GetIndiciesCount());
      r.d3d.SetPixelShaderResource(mesh.GetDiffuseMap());
      r.d3d.RenderPass();
      r.d3d.AfterRenderPass();
    }
  }
}
