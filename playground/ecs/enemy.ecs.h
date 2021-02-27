// *************************************************************
// File:    enemy.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_ENEMY_H
#define AH_ECS_COMP_ENEMY_H

#include <ecs/component.h>
#include <ecs/manager.h>

#include <system/rand_utils.h>

#include <math/vector3.h>

#include <render/camera_eul.h>

#include "transform.ecs.h"
#include "render.ecs.h"
#include "input.ecs.h"
#include "camera.ecs.h"
#include "stat.ecs.h"
#include "physics.ecs.h"
#include "target.ecs.h"
#include "gun.ecs.h"

using namespace gdm;

struct Ai : ecs::Component<ECS_COMPONENT_IDX | Target::Sig()>
{
  Target target;
};

static const float k_world_xyz {10.f};
static ecs::EntityManager& s_mgr = ecs::EntityManager::GetInstance();

// ECS_CONDITION(stat.enemy_spawned)
static const char* k_enemy_model {"../../_models_old/cube_test/cube_test.ply"}; // todo: load once
static const int k_enemy_cnt {20};

static void enemy_spawn(Statistic& stat)
{
  if (stat.enemy_spawned)
    return;

  for (int i = 0; i < k_enemy_cnt; ++i)
  {
    const float x = rand::GetRand(-k_world_xyz, k_world_xyz);
    const float y = rand::GetRand(-k_world_xyz, k_world_xyz);
    const float z = rand::GetRand(-k_world_xyz, k_world_xyz);
    Mat4f model_mat (1.f);
    model_mat.SetCol(3, Vec3f(x,y,z));

    ecs::EntityId eid = s_mgr.CreateEntity(
      std::make_tuple(
        Renderable { k_enemy_model },
        Transform  { model_mat },
        Ai { },
        Physics { vec3::Normalize(Vec3f{x,y,z}) },
        Gun { },
        (Camera*)nullptr
      ));
  }
  stat.enemy_spawned = true;
}

void enemy_set_velocity(Transform& tm, Physics& phys, Ai& ai)
{
  if(ai.target.IsAquired())
  {
    Vec3f dir = ai.target.GetPos() - tm->GetCol(3);
    phys.SetStuntVelocity(vec3::Normalize(dir));
  }
  else
    phys.SetCruiseVelocity();
}

void enemy_move(ecs::Dt dt, Transform& tm, Physics& phys, const Ai& /* ai */)
{
  Vec3f pos = tm->GetCol(3);
  pos += phys.GetVelocity() * dt.Get();
  tm->SetCol(3, pos);
  // todo: look at target
}

void enemy_attack(const Transform& tm, Physics& phys, Gun& gun, const Ai& ai)
{
  if(!ai.target.IsAquired())
    return;
  if(gun.temp > gun.k_max_temp || gun.time_from_last_shot < gun.k_min_shot_time)
    return;

  Vec3f aim = tm->GetCol(3) - ai.target.GetPos();
  aim.Normalize();
  gun.time_from_last_shot = 0.f;
}

const float k_enemy_detect = 10.f;
void enemy_search_target(const Transform& tm, const Camera& cam, Ai& ai)
{
  Vec3f dist_v = tm->GetCol(3) - cam->GetPos();
  if(dist_v.SqLength() < k_enemy_detect * k_enemy_detect)
    ai.target.Aquire(cam->GetPos());
  else
    ai.target.Clear();
}

void enemy_player_collision(const ecs::Eid& eid, const Transform& tm, const Camera& cam, Ai& /*ai*/)
{
  Vec3f dist_v = tm->GetCol(3) - cam->GetPos();
  if (dist_v.SqLength() < 2.f)
    s_mgr.DeleteEntity(eid.Get());
}

#endif // AH_ECS_COMP_ENEMY_H
