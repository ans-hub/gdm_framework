// *************************************************************
// File:    helpers.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "helpers.h"

#include "data/cfg_loader.h"
#include "memory/defines.h"
#include "render/api.h"
#include "system/assert_utils.h"

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

  auto LoadModelsByPathes(const std::vector<std::string>& pathes) -> std::vector<ModelHandle>
  {
    std::vector<ModelHandle> result;
    for (std::size_t i = 0; i < pathes.size(); ++i)
    {
      const char* model_fpath = pathes[i].c_str();
      ModelHandle model_handle = {};
      if (ModelFactory::Has(model_fpath))
        model_handle = ModelFactory::GetHandle(model_fpath);
      else
        model_handle = ModelFactory::Load(model_fpath);
      result.push_back(model_handle);
    }
    return result;
  }
}

// --public

auto gdm::helpers::LoadLamps(const Config& cfg) -> std::vector<ModelHandle>
{
  static const bool registered = _private::RegisterFactoryPathes(cfg);

  auto lamp_pathes = cfg.GetAllVals<std::string>("lamp_");
  auto lamp_poses = cfg.GetAllVals<Vec4f>("lamp_pos_");
  auto lamp_colors = cfg.GetAllVals<Vec4f>("lamp_col_");
  auto lamp_dirs = cfg.GetAllVals<Vec4f>("lamp_dir_");
  ASSERT(lamp_pathes.size() == lamp_poses.size());
  ASSERT(lamp_pathes.size() == lamp_colors.size());
  ASSERT(lamp_pathes.size() == lamp_dirs.size());

  std::vector<ModelHandle> result = _private::LoadModelsByPathes(lamp_pathes);
  ASSERT(lamp_pathes.size() == result.size());

  for (std::size_t i = 0; i < lamp_poses.size(); ++i)
  {
    AbstractModel* model = ModelFactory::Get(result[i]);
    model->tm_ = Mat4f(1.f);
    model->tm_.SetCol(3, lamp_poses[i].xyz());
    Mat4f tm = matrix::MakeScale(lamp_poses[i][3]) % model->tm_;
    model->tm_ = tm;
    model->color_ = lamp_colors[i];
  }
  return result;
}

auto gdm::helpers::LoadObjects(const Config& cfg) -> std::vector<ModelHandle>
{
  static const bool registered = _private::RegisterFactoryPathes(cfg);

  auto obj_pathes = cfg.GetAllVals<std::string>("model_");
  auto obj_poses = cfg.GetAllVals<Vec4f>("model_pos_");
  ASSERT(obj_pathes.size() == obj_poses.size());

  std::vector<ModelHandle> result = _private::LoadModelsByPathes(obj_pathes);
  ASSERT(obj_pathes.size() == result.size());

  for (std::size_t i = 0; i < obj_poses.size(); ++i)
  {
    AbstractModel* model = ModelFactory::Get(result[i]);
    model->tm_ = Mat4f(1.f);
    model->tm_.SetCol(3, obj_poses[i].xyz());
    Mat4f tm = matrix::MakeScale(obj_poses[i][3]) % model->tm_;
    model->tm_ = tm;
  }
  return result;
}

auto gdm::helpers::MergeObjects(const std::vector<ModelHandle>& objs, const std::vector<ModelHandle>& lamps) -> std::vector<gdm::ModelHandle>
{
  std::set<ModelHandle> to_sort;
  for (auto handle : objs)
    to_sort.emplace(handle);
  for (auto handle : lamps)
    to_sort.emplace(handle);
  std::vector<ModelHandle> result;
  for(auto handle : to_sort)
    result.push_back(handle);
  return result;
}

void gdm::helpers::UpdateCamera(CameraEul& cam, MainInput& input, float dt)
{
  cam.Rotate(input.GetMouseY(), input.GetMouseX());

  dt += dt * static_cast<int>(input.IsKeyboardBtnHold(DIK_LSHIFT)) * 2;

  if (input.IsKeyboardBtnHold(DIK_W))
    cam.Move(-cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_S))
    cam.Move(cam.GetTm().GetCol(0), dt);
  if (input.IsKeyboardBtnHold(DIK_A))
    cam.Move(-cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_D))
    cam.Move(cam.GetTm().GetCol(2), dt);
  if (input.IsKeyboardBtnHold(DIK_R))
    cam.Move(cam.GetTm().GetCol(1), dt);
  if (input.IsKeyboardBtnHold(DIK_F))
    cam.Move(-cam.GetTm().GetCol(1), dt);
}
