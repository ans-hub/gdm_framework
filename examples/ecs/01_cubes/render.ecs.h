// *************************************************************
// File:    render.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_RENDER_H
#define AH_ECS_COMP_RENDER_H

#include <memory>

#include <ecs/component.h>

#include <render/dx11/dx_renderer.h>
#include <render/dx11/dx_object.h>

#include "window.ecs.h"
#include "camera.ecs.h"
#include "transform.ecs.h"

using namespace gdm;

struct Window;
struct Camera;
struct RenderPimpl;

struct Render : ecs::SingletonComponent<ECS_COMPONENT_IDX | Camera::Sig() | Window::Sig()>
{
  Render();
  ~Render();
  Window& window;
  Camera& camera;
  DxRenderer d3d;
  std::unique_ptr<RenderPimpl> impl;
};

struct Renderable : ecs::Component<ECS_COMPONENT_IDX>
{
  Renderable() =default;
  Renderable(const char* model_fpath);
  
  DxObject object;
  ECS_DEFINE_ACCESS_OPERATORS(object)
};

void render_begin_frame(Render& r);
void render_end_frame(Render& r);
void render_pass_main_prepare(Render& r);
void render_pass_main_set_camera(Render& r, const Camera& camera);
void render_pass_main_prepare_objects(Renderable& object, Transform& tm);
void render_pass_main_sort_objects(Render& r, const Camera& cam);
void render_pass_main_draw_objects(Render& r);

#endif // AH_ECS_COMP_RENDER_H
