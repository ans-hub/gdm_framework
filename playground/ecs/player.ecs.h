// *************************************************************
// File:    player.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_PLAYER_H
#define AH_ECS_COMP_PLAYER_H

#include <ecs/component.h>
#include <ecs/manager.h>

#include <engine/camera_eul.h>

#include "transform.ecs.h"
#include "input.ecs.h"
#include "camera.ecs.h"
#include "stat.ecs.h"

using namespace gdm;

// ECS_CONDITION(!stat.player_spawned)
const Vec3f k_player_pos = Vec3f{0.f, 0.f, 0.f};
static void player_spawn(Statistic& stat)
{
  if(stat.player_spawned)
    return;

  Mat4f model_mat (1.f);
  model_mat.SetCol(3, k_player_pos);
  ecs::EntityId eid = ecs::EntityManager::GetInstance().CreateEntity<Transform&&, Camera*, Input*>({
    Transform{ model_mat },
    nullptr,
    nullptr // ECS_SINGLETON_COMP_INIT
  });
  stat.player_spawned = true;
}

inline void player_move(ecs::Dt dt, Camera& cam, Input& input)
{
  input->Capture();

  cam->Rotate(input->GetMouseY(), input->GetMouseX());
  if (input->IsKeyboardBtnHold(DIK_W))
    cam->Move(-cam->GetTm().GetCol(0), dt.Get());
  if (input->IsKeyboardBtnHold(DIK_S))
    cam->Move(cam->GetTm().GetCol(0), dt.Get());
  if (input->IsKeyboardBtnHold(DIK_A))
    cam->Move(-cam->GetTm().GetCol(2), dt.Get());
  if (input->IsKeyboardBtnHold(DIK_D))
    cam->Move(cam->GetTm().GetCol(2), dt.Get());
  if (input->IsKeyboardBtnHold(DIK_R))
    cam->Move(cam->GetTm().GetCol(1), dt.Get());
  if (input->IsKeyboardBtnHold(DIK_F))
    cam->Move(-cam->GetTm().GetCol(1), dt.Get());
}

#endif // AH_ECS_COMP_PLAYER_H
