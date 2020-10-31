// *************************************************************
// File:    camera_eul.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "camera_eul.h"

gdm::CameraEul::CameraEul(float fov, float ar, float z_near, float z_far)
  : tm_(matrix::MakeLHBasis())
  , itm_(1.f)
  , proj_tm_ {matrix::MakePerspectiveLH(fov, ar, z_near, z_far)}
  , fov_{fov}
  , ar_{ar}
  , z_near_{z_near}
  , z_far_{z_far}
  , axis_{0.f, 1.f, 0.f}
  , speed_{0.005f}
{ }

void gdm::CameraEul::PrepareItm()
{
  Mat4f cw_tm = matrix::RotateBasisYCW(tm_);
  itm_ = matrix::InverseTransform(cw_tm);
}

void gdm::CameraEul::SetOrient(const Mat4f& tm)
{
  tm_.SetCol(0, tm.GetCol(0));
  tm_.SetCol(1, tm.GetCol(1));
  tm_.SetCol(2, tm.GetCol(2));
  PrepareItm();
}

void gdm::CameraEul::SetPos(Vec3f pos)
{
  tm_.SetCol(3, pos);
  PrepareItm();
}

void gdm::CameraEul::LookAt(const Vec3f& look_at)
{
  matrix::MakeLookAt(tm_, look_at, tm_.GetCol(3), tm_.GetCol(1));
  PrepareItm();
}

void gdm::CameraEul::Move(const Vec3f& dir, float dt)
{
  Vec3f vel = dir * speed_ * dt;
  tm_.SetCol(3, tm_.GetCol(3) + vel);
  PrepareItm();
}

void gdm::CameraEul::SetAspectRatio(float ar)
{
  ASSERT(fov_ > 0.f);
  ASSERT(z_near_ > 0.f);
  ASSERT(z_far_ > z_near_);

  ar_ = ar;
  proj_tm_ = matrix::MakePerspectiveLH(fov_, ar_, z_near_, z_far_);
}

void gdm::CameraEul::Rotate(float yaw, float pitch)
{
  Mat4f rot_y = matrix::MakeRotateY(yaw);
  Mat4f rot_p = matrix::MakeRotateX(pitch);
  Vec3f pos = tm_.GetCol(3);
  tm_ = rot_y % rot_p % matrix::MakeLHBasis();
  tm_.SetCol(3, pos); // todo: % doesn't work; 27.05 - already work
  PrepareItm();
}
