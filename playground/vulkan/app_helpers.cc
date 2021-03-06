// *************************************************************
// File:    app_helpers.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "app_helpers.h"

#include "data/cfg_loader.h"

#include "memory/defines.h"

#include "render/api.h"

#include "system/assert_utils.h"
#include "system/diff_utils.h"

//--public

namespace gdm::_private{

  bool RegisterFactoryPathes(const gdm::Config& cfg)
  {
    static bool v_registered = false;

    if (v_registered)
      return true;

    std::string models_path = cfg.Get<std::string>("models_factory_path");
    std::string materials_path = cfg.Get<std::string>("material_factory_path");
    std::string textures_path = cfg.Get<std::string>("texture_factory_path");
    std::string images_path = cfg.Get<std::string>("image_factory_path");
    
    gdm::MaterialFactory::SetPath(materials_path.c_str());
    gdm::TextureFactory::SetPath(textures_path.c_str());
    gdm::ImageFactory::SetPath(images_path.c_str());
    gdm::ModelFactory::SetPath(models_path.c_str());

    v_registered = true;
    return true;
  }

  auto LoadModelsByPathes(const std::vector<std::string>& pathes) -> std::vector<ModelInstance>
  {
    std::vector<ModelInstance> result;
    
    for (const auto& path: pathes)
    {
      ModelHandle model_handle = {};

      if (ModelFactory::Has(path.c_str()))
        model_handle = ModelFactory::GetHandle(path.c_str());
      else
        model_handle = ModelFactory::Load(path.c_str());

      ModelInstance instance;
      instance.handle_ = model_handle;
      instance.tm_ = Mat4f(1.f);
      instance.color_ = Vec4f(-1.f);
      result.push_back(instance);
    }
    return result;
  }
}

// --public

auto gdm::app_helpers::LoadFlashlights(const Config& cfg) -> std::vector<ModelInstance>
{
  static const bool registered = _private::RegisterFactoryPathes(cfg);

  auto flashlight_path = cfg.GetAllVals<std::string>("flashlight_");
  auto flashlight_colors = cfg.GetAllVals<Vec4f>("flashlight_col_");
  ASSERT(flashlight_path.size() == flashlight_colors.size());

  std::vector<ModelInstance> result = _private::LoadModelsByPathes(flashlight_path);

  for (std::size_t i = 0; i < result.size(); ++i)
  {
    result[0].color_ = flashlight_colors[0];
    result[0].color_.w = static_cast<float>(LightType::SPOTLIGHT);
  }
  
  return result;
}

auto gdm::app_helpers::LoadLights(const Config& cfg) -> std::vector<ModelInstance>
{
  static const bool registered = _private::RegisterFactoryPathes(cfg);

  auto lamp_pathes = cfg.GetAllVals<std::string>("lamp_");
  auto lamp_names = cfg.GetAllKeys<std::string>("lamp_");
  auto lamp_poses = cfg.GetAllVals<Vec4f>("lamp_pos_");
  auto lamp_colors = cfg.GetAllVals<Vec4f>("lamp_col_");
  auto lamp_dirs = cfg.GetAllVals<Vec4f>("lamp_dir_");

  ASSERT(lamp_pathes.size() == lamp_poses.size());
  ASSERT(lamp_pathes.size() == lamp_colors.size());
  ASSERT(lamp_pathes.size() == lamp_dirs.size());

  std::vector<ModelInstance> result = _private::LoadModelsByPathes(lamp_pathes);
  ASSERT(lamp_pathes.size() == result.size());

  for (std::size_t i = 0; i < lamp_poses.size(); ++i)
  {
    ModelInstance& instance = result[i];
    Vec3f fwd = vec3::Normalize(lamp_dirs[i].xyz());
    Vec3f up = Vec3f(0,1,0);
    float upfwd_dot = vec3::DotProduct(fwd,up);
    if (upfwd_dot > 1.f - std::numeric_limits<float>::epsilon())
      up = Vec3f(0,0,-1);
    else if(upfwd_dot < -1.f + std::numeric_limits<float>::epsilon())
      up = Vec3f(0,0,1);
    Vec3f right = -vec3::Normalize(fwd % up);
    up = -vec3::Normalize(right % fwd);
    instance.tm_.SetCol(0, right);
    instance.tm_.SetCol(1, up);
    instance.tm_.SetCol(2, fwd);
    Mat4f tm_tmp = matrix::MakeScale(lamp_poses[i][3]) % instance.tm_;
    instance.tm_ = tm_tmp;
    instance.tm_.SetCol(3, lamp_poses[i].xyz());
    instance.color_ = lamp_colors[i];
  }
  return result;
}

auto gdm::app_helpers::LoadObjects(const Config& cfg) -> std::vector<ModelInstance>
{
  static const bool registered = _private::RegisterFactoryPathes(cfg);

  auto obj_pathes = cfg.GetAllVals<std::string>("model_");
  auto obj_names = cfg.GetAllKeys<std::string>("model_");
  auto obj_poses = cfg.GetAllVals<Vec4f>("model_pos_");

  ASSERT(obj_pathes.size() == obj_poses.size());

  std::vector<ModelInstance> models = _private::LoadModelsByPathes(obj_pathes);

  ASSERT(obj_pathes.size() == models.size());

  for (std::size_t i = 0; i < obj_poses.size(); ++i)
  {
    ModelInstance& instance = models[i];
    instance.tm_.SetCol(3, obj_poses[i].xyz());
    Mat4f tm = instance.tm_ * matrix::MakeScale(obj_poses[i][3]);
    instance.tm_ = tm;
    instance.color_.w = -1.f;
  }
  return models;
}

auto gdm::app_helpers::LoadObjectNames(const Config& cfg) -> std::vector<std::string>
{
  return cfg.GetAllKeys<std::string>("model_");
}

auto gdm::app_helpers::GetUniqueModels(const std::vector<ModelInstance*>& instances) -> std::vector<gdm::ModelHandle>
{
  std::set<ModelHandle> to_sort;
  for (const auto& instance : instances)
    to_sort.emplace(instance->handle_);
  std::vector<ModelHandle> result;
  for(auto handle : to_sort)
    result.push_back(handle);
  return result;
}
