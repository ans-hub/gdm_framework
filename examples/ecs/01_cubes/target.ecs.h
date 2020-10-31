// *************************************************************
// File:    target.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_TARGET_H
#define AH_ECS_COMP_TARGET_H

#include <ecs/component.h>

#include <math/vector3.h>

using namespace gdm;

struct Target : ecs::Component<ECS_COMPONENT_IDX>
{
  bool IsAquired() const { return aquired; }
  void Aquire(const Vec3f& pos) { target = pos; aquired = true; }
  void Clear() { aquired = false; }
  const Vec3f& GetPos() const { return target; }

  bool aquired = false;
  Vec3f target;
};

#endif // AH_ECS_COMP_TARGET_H
