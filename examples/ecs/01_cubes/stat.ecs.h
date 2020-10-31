// *************************************************************
// File:    stat.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_STAT_H
#define AH_ECS_COMP_STAT_H

#include <ecs/component.h>

struct Statistic : ecs::SingletonComponent<ECS_COMPONENT_IDX>
{
  bool enemy_spawned = false;
  bool player_spawned = false;
};

#endif // AH_ECS_COMP_STAT_H
