// *************************************************************
// File:    gun.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_GUN_H
#define AH_ECS_COMP_GUN_H

#include <algorithm>

#include <ecs/component.h>

#include <math/vector3.h>

using namespace gdm;

struct Gun : ecs::Component<ECS_COMPONENT_IDX>
{
  float temp = 0.f;
  constexpr static float k_max_temp = 10.f;
  constexpr static float k_shot_temp = 2.f;
  constexpr static float k_cold_per_sec = 2.f;

  float time_from_last_shot = 0.f;
  constexpr static float k_min_shot_time = 0.1f;
  constexpr static float k_shot_speed = 10.f;
};

inline void update_gun(ecs::Dt dt, Gun& gun)
{
  gun.temp = std::max<float>(0.f, gun.temp - gun.k_cold_per_sec * dt.Get());
  gun.time_from_last_shot += dt.Get();
}

#endif // AH_ECS_COMP_GUN_H
