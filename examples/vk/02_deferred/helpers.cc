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

// --public

auto gdm::helpers::LoadAbstractModels(const Config& cfg) -> std::vector<ModelHandle>
{
  std::vector<ModelHandle> result;

  auto obj_pathes = cfg.GetAllVals<std::string>("model_");
  auto obj_poses = cfg.GetAllVals<Vec4f>("model_pos_");

  ASSERT(obj_pathes.size() == obj_poses.size());

  for (std::size_t i = 0; i < obj_pathes.size(); ++i)
  {
    const char* model_fpath = obj_pathes[i].c_str();
    ModelHandle model_handle = {};
    if (ModelFactory::Has(model_fpath))
      model_handle = ModelFactory::GetHandle(model_fpath);
    else
      model_handle = ModelFactory::Load(model_fpath);
    result.push_back(model_handle);
  }
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
