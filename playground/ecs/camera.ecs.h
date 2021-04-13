// *************************************************************
// File:    camera.ecs.h
// Author:  Novoselov Anton @ 2018
// Descr:   camera ecs component
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_CAMERA_H
#define AH_ECS_COMP_CAMERA_H

#include <math/vector3.h>

#include <ecs/component.h>

#include <engine/camera_eul.h>

using namespace gdm;

struct Camera : ecs::SingletonComponent<ECS_COMPONENT_IDX>
{
  Camera::Camera(float fov, float ar, float z_near, float z_far)
    : camera(fov, ar, z_near, z_far)
  {
    camera.SetPos({0.f, 0.f, 0.f});
    camera.SetMoveSpeed(2.f);
  }
  CameraEul camera;
  ECS_DEFINE_ACCESS_OPERATORS(camera)
};

#endif // AH_ECS_COMP_CAMERA_H