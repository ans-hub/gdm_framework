// *************************************************************
// File:    camera_eul.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GL_CAMERA_EUL_H
#define AH_GL_CAMERA_EUL_H

#include <math/matrix.h>
#include <math/vector2.h>
#include <math/vector3.h>

#include <system/assert_utils.h>

namespace gdm {

struct CameraEul
{
  Mat4f tm_;
  Mat4f itm_;
  Mat4f proj_tm_;
  float fov_;
  float ar_;
  float z_near_;
  float z_far_;
  Vec3f axis_;
  float speed_;

public:
  CameraEul(float fov, float ar, float z_near, float z_far);

  const Mat4f& GetTm() const { return tm_; }
  const Vec3f GetPos() const { return tm_.GetCol(3); }
  const float GetAspectRatio() const { return ar_; }
  const Mat4f& GetViewMx() const { return itm_; }
  const Mat4f& GetProjectionMx() const { return proj_tm_; }

  void SetOrient(const Mat4f& tm);
  void SetPos(Vec3f pos);
  void SetMoveSpeed(float speed) { speed_ = speed; }
  void Move(const Vec3f& dir, float dt);
  void LookAt(const Vec3f& look_at);
  void SetAspectRatio(float ar);
  void Rotate(float yaw, float pitch);
  void Rotate(const Vec3f& axis, float angle);

private:
  void PrepareItm();

}; // struct CameraEul

} // namespace gdm

#endif // AH_GL_CAMERA_EUL_H