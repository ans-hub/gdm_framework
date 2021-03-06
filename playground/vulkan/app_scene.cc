// *************************************************************
// File:    app_scene.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "app_scene.h"

#include "system/diff_utils.h"

#include "render/shader.h"
#include "render/desc/sampler_desc.h"
#include "render/desc/input_layout_desc.h"
#include "render/desc/rasterizer_desc.h"
#include "engine/debug_draw.h"

#include "app_input.h"
#include "app_helpers.h"
#include "app_defines.h"

// --public

gdm::AppScene::AppScene(Config& cfg, MainWindow& win)
  : camera_{cfg::v_fov, win.GetAspectRatio(), cfg::v_znear, cfg::v_zfar}
  , models_{}
  , models_names_{}
  , flashlights_{}
  , lamps_{}
  , renderable_materials_{}
  , dummy_view_{}
  , cfg_dispatcher_{}
{
  camera_.SetPos(cfg.Get<Vec3f>("initial_cam_pos"));  // default value?
  camera_.LookAt(cfg.Get<Vec3f>("initial_look_at"));
  camera_.SetMoveSpeed(3.f);

  std::vector<ModelInstance> object_models = app_helpers::LoadObjects(cfg);
  std::vector<ModelInstance> lamp_models = app_helpers::LoadLights(cfg);
  std::vector<ModelInstance> flashlights = app_helpers::LoadFlashlights(cfg);

  SetObjects(object_models, app_helpers::LoadObjectNames(cfg));
  SetLamps(lamp_models, flashlights);

  cfg_dispatcher_ = cfg::Dispatcher(cfg, GetSceneInstances(), GetSceneInstancesNames());
}

void gdm::AppScene::Update(float dt,
  AppInput& input_mgr,
  GuiManager& gui,
  DebugDraw& debug_draw)
{
  input_mgr.GetRawInput().CaptureKeyboard();
  
  if (!gui.IsActiveAnyWindow())
  {
    input_mgr.GetRawInput().CaptureMouse();
    input_mgr.UpdateCameraRotate(camera_, dt);
  }
  else
    input_mgr.GetRawInput().PauseCaptureMouse();

  input_mgr.UpdateCameraMovement(camera_, dt);
  input_mgr.UpdateLamps(camera_, GetLamps(), dt);
  input_mgr.UpdateFlashlights(camera_, GetFlashlights(), dt);
  input_mgr.UpdateDebugDraw(debug_draw);
  input_mgr.UpdateGui(gui);

  cfg_dispatcher_.Update(camera_, input_mgr.GetRawInput(), debug_draw, dt);
}

void gdm::AppScene::SetObjects(const std::vector<ModelInstance>& objs, const std::vector<std::string>& names)
{
  for (auto [instance, name] : range::ZipSpan(objs, names))
  {
    models_.push_back(instance);
    models_names_.push_back(name);
  }
}

void gdm::AppScene::SetLamps(const std::vector<ModelInstance>& lamps, const std::vector<ModelInstance>& flashlights)
{
  for (auto instance : lamps)
  {
    models_.push_back(instance);
    models_names_.push_back("Light");

    lamps_.push_back({});
    lamps_.back().instance_ = instance;
    lamps_.back().enabled_ = true;
  }
  for (auto instance : flashlights)
  {
    flashlights_.push_back({});
    flashlights_.back().instance_ = instance;
    flashlights_.back().enabled_ = true;
  }
}

auto gdm::AppScene::GetRenderableInstances() -> std::vector<ModelInstance*>
{
  std::vector<ModelInstance*> renderables;
 
  for (auto& instance : models_)
    renderables.push_back(&instance);
    
  return renderables;
}

auto gdm::AppScene::GetSceneInstances() -> std::vector<ModelInstance*>
{
  std::vector<ModelInstance*> scene_models;
 
  for (auto& instance : models_)
    scene_models.push_back(&instance);
    
  return scene_models;
}

auto gdm::AppScene::GetSceneInstancesNames() const -> const std::vector<std::string>&
{
  return models_names_;
}

auto gdm::AppScene::GetRenderableMaterials() -> const api::ImageViews&
{
  if (!dummy_view_)
  {
    TextureHandle dummy_handle = dummy_handle = TextureFactory::GetHandle(cfg::v_dummy_image);
    AbstractTexture* dummy_texture = TextureFactory::Get(dummy_handle);
    dummy_view_ = &dummy_texture->GetTextureImpl().GetImageViewImpl();
  }

  renderable_materials_.clear();
  renderable_materials_.resize(cfg::v_max_materials * cfg::v_material_type_cnt, dummy_view_);
  
  std::vector<ModelInstance*> renderable = GetRenderableInstances();

  for (auto [index,model_instance] : Enumerate(renderable))
  {
    auto model = ModelFactory::Get(model_instance->handle_);
    for (auto mesh_handle : model->meshes_)
    {
      auto mesh = MeshFactory::Get(mesh_handle);
      auto material = MaterialFactory::Get(mesh->material_);
      auto diffuse_texture = TextureFactory::Get(material->diff_);
      auto normal_texture = TextureFactory::Get(material->norm_);
      auto specular_texture = TextureFactory::Get(material->spec_);
      
      renderable_materials_[material->index_ * 3 + cfg::v_diff_offset] = &(diffuse_texture->GetTextureImpl().GetImageViewImpl());
      renderable_materials_[material->index_ * 3 + cfg::v_norm_offset] = &(normal_texture->GetTextureImpl().GetImageViewImpl());
      renderable_materials_[material->index_ * 3 + cfg::v_spec_offset] = &(specular_texture->GetTextureImpl().GetImageViewImpl());
    }
  }
  return renderable_materials_;
}
