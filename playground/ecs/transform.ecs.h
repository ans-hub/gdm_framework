// *************************************************************
// File:    transform.ecs.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_TM_H
#define AH_ECS_COMP_TM_H

#include <ecs/component.h>

#include <math/matrix.h>

using namespace gdm;

struct Transform : ecs::Component<ECS_COMPONENT_IDX>
{
  Transform() =default;
  Transform(const Mat4f& m) : mat{m} { }
  Mat4f mat;
  ECS_DEFINE_ACCESS_OPERATORS(mat)
};

#endif // AH_ECS_COMP_TM_H
