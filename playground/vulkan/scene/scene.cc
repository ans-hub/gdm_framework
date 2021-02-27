// *************************************************************
// File:    scene.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "scene.h"

#include "system/diff_utils.h"

#include "render/shader.h"

#include "render/desc/sampler_desc.h"
#include "render/desc/input_layout_desc.h"
#include "render/desc/rasterizer_desc.h"

#include "helpers.h"

// --public

gdm::Scene::Scene(Config& cfg, MainWindow& win)
  : camera_{v_fov, win.GetAspectRatio(), v_znear, v_zfar}
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

  std::vector<ModelInstance> object_models = helpers::LoadObjects(cfg);
  std::vector<ModelInstance> lamp_models = helpers::LoadLights(cfg);
  std::vector<ModelInstance> flashlights = helpers::LoadFlashlights(cfg);

  SetObjects(object_models, helpers::LoadObjectNames(cfg));
  SetLamps(lamp_models, flashlights);

  cfg_dispatcher_ = cfg::Dispatcher(cfg, GetSceneInstances(), GetSceneInstancesNames());
}

void gdm::Scene::Update(MainInput& input, DebugDraw& debug_draw, float dt)
{
  helpers::UpdateCamera(camera_, input, dt);
  helpers::UpdateLamps(camera_, input, GetLamps(), dt);
  helpers::UpdateFlashlights(camera_, input, GetFlashlights(), dt);

  cfg_dispatcher_.Update(camera_, input, debug_draw, dt);
}

void gdm::Scene::SetObjects(const std::vector<ModelInstance>& objs, const std::vector<std::string>& names)
{
  for (auto [instance, name] : range::ZipSpan(objs, names))
  {
    models_.push_back(instance);
    models_names_.push_back(name);
  }
}

void gdm::Scene::SetLamps(const std::vector<ModelInstance>& lamps, const std::vector<ModelInstance>& flashlights)
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

auto gdm::Scene::GetRenderableInstances() -> std::vector<ModelInstance*>
{
  std::vector<ModelInstance*> renderables;
 
  for (auto& instance : models_)
    renderables.push_back(&instance);
    
  return renderables;
}

auto gdm::Scene::GetSceneInstances() -> std::vector<ModelInstance*>
{
  std::vector<ModelInstance*> scene_models;
 
  for (auto& instance : models_)
    scene_models.push_back(&instance);
    
  return scene_models;
}

auto gdm::Scene::GetSceneInstancesNames() const -> const std::vector<std::string>&
{
  return models_names_;
}

auto gdm::Scene::GetRenderableMaterials() -> const api::ImageViews&
{
  if (!dummy_view_)
  {
    TextureHandle dummy_handle = dummy_handle = TextureFactory::GetHandle(v_dummy_image);
    AbstractTexture* dummy_texture = TextureFactory::Get(dummy_handle);
    dummy_view_ = dummy_texture->GetApiImageView<api::ImageView>();
  }

  renderable_materials_.clear();
  renderable_materials_.resize(v_max_materials * v_material_type_cnt, dummy_view_);
  
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
      
      renderable_materials_[material->index_ * 3 + v_diff_offset] = diffuse_texture->GetApiImageView<api::ImageView>();
      renderable_materials_[material->index_ * 3 + v_norm_offset] = normal_texture->GetApiImageView<api::ImageView>();
      renderable_materials_[material->index_ * 3 + v_spec_offset] = specular_texture->GetApiImageView<api::ImageView>();
    }
  }
  return renderable_materials_;
}
