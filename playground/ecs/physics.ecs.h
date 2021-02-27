// *************************************************************
// File:    physic.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_PHYSICS_H
#define AH_ECS_COMP_PHYSICS_H

#include <ecs/component.h>

#include <math/vector3.h>

using namespace gdm;

struct Physics : ecs::Component<ECS_COMPONENT_IDX>
{
  Physics() =default;
  Physics(Vec3f vel) : velocity{vel} {}
  void SetCruiseVelocity() { velocity.Normalize() * 0.5f; }
  void SetStuntVelocity(const Vec3f& dir) { velocity = dir * 2.f; }
  const Vec3f& GetVelocity() const { return velocity; }
  Vec3f velocity;
};

#endif // AH_ECS_COMP_PHYSICS_H
