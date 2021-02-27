// *************************************************************
// File:    tennis.cfg.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <string>
#include <vector>
#include <unordered_map>

#include <data/model_factory.h>
#include <render/camera_eul.h>
#include <render/debug_draw.h>
#include <window/main_input.h>
#include <system/diff_utils.h>
#include <math/vector3.h>
#include <math/general.h>
#include <math/obb.h>
#include <math/intersection.h>
#include <math/sphere.h>

#include "scene/types.h"

//--private

namespace gdm::_private
{
  static const Vec3f v_g (0.f, -3.f, 0.f);
  static const float v_ball_max_y = -3.f; 
  static const float v_table_linear_speed = 4.f;
  static const float v_table_angle_speed = 45.f;
  static const float v_ball_linear_speed = 1.f;

  struct Table
  {
    Table(Mat4f& tm)
      : tm_{tm}
      , size_{matrix::DecomposeToScale(tm).x}
      , vel_{}
      , bounding_box_{Vec3f{size_/2.f, size_/2.f, size_/2.f}, tm_}
    { }

    Mat4f& tm_;
    float size_;
    Vec3f vel_;
    OBB bounding_box_;
  };

  struct Ball
  {
    Ball(Mat4f& tm)
      : tm_{tm}
      , size_{matrix::DecomposeToScale(tm).x}
      , vel_(0.f, -v_ball_linear_speed, 0.f)
      , accel_(0.f)
      , bounding_sphere_{size_/2.f, tm_.GetColRef(3)}
    { }

    Mat4f& tm_;
    float size_;
    Vec3f vel_;
    Vec3f accel_;
    Sphere bounding_sphere_;
  };

  static bool ResetBall(Ball& ball)
  {
    static Vec3f initial_pos = ball.tm_.GetCol(3);
    static Vec3f initial_vel = ball.vel_;
    static Vec3f initial_accel = v_g;
    
    ball.tm_.SetCol(3, initial_pos);
    ball.vel_ = initial_vel;
    ball.accel_ = initial_accel;

    return true;
  }

  static void UpdateBall(Ball& ball, Table& table, DebugDraw& debug, float dt)
  {
    if (dt < math::kEpsilon)
      return;

    if (ball.tm_.GetCol(3).y < v_ball_max_y)
      ResetBall(ball);

    phys::CollisionManifold col {};

    if (phys::IsIntersects(ball.bounding_sphere_, table.bounding_box_, col))
    {
      Vec3f pen_dir = vec3::Normalize(ball.tm_.GetCol(3) - col.contact_points[0]);
      Vec3f corr_pos = ball.tm_.GetCol(3);
      corr_pos += (pen_dir * col.penetration);
      ball.tm_.SetCol(3, corr_pos);
      ball.vel_ = vec3::Reflect(ball.vel_, col.normal);
      ball.vel_ += table.vel_;
      Vec3f ball_pos = ball.tm_.GetCol(3);
      ball_pos += ball.vel_ * dt;
      ball.tm_.SetCol(3, ball_pos);
    }
    else
    {
      Vec3f curr_vel = ball.vel_ * dt;
      ball.vel_ += ball.accel_ * dt;
      Vec3f ball_pos = ball.tm_.GetCol(3);
      ball_pos += curr_vel;
      ball.tm_.SetCol(3, ball_pos);
    }

    debug.DrawSphere(ball.tm_.GetColRef(3), ball.bounding_sphere_.radius_, color::LightGray);
    debug.DrawCross(col.closest_point, 0.05f, color::LightGray);
  }

  static void UpdateTable(Table& table, MainInput& input, DebugDraw& debug, float dt)
  {
    Vec3f fwd = table.tm_.GetCol(2);
    float vel_sign = 0.f;
    
    if (input.IsKeyboardBtnHold(DIK_UP))
      vel_sign = 1.f;
    else if (input.IsKeyboardBtnHold(DIK_DOWN))
      vel_sign = -1.f;
    
    table.vel_ = vec3::Normalize(fwd) * v_table_linear_speed * dt * vel_sign;
    Vec3f pos = table.tm_.GetCol(3);
    pos += table.vel_;
    table.tm_.SetCol(3, pos);

    float angle_y = 0.f;
    float angle_z = 0.f;

    if (input.IsKeyboardBtnHold(DIK_LEFT))
    {
      if (input.IsKeyboardBtnHold(DIK_LSHIFT))
        angle_z = -1.f * v_table_angle_speed * dt;
      else
        angle_y = -1.f * v_table_angle_speed * dt;
    }
    else if (input.IsKeyboardBtnHold(DIK_RIGHT))
    {
      if (input.IsKeyboardBtnHold(DIK_LSHIFT))
        angle_z = 1.f * v_table_angle_speed * dt;
      else
        angle_y = 1.f * v_table_angle_speed * dt;
    }

    Mat4f roty (1.f);
    Mat4f rotz (1.f);
    
    if (std::abs(angle_y) > math::kEpsilon)
      roty = matrix::MakeRotateY(angle_y);
    if (std::abs(angle_z) > math::kEpsilon)
      rotz = matrix::MakeRotateZ(angle_z);

    table.tm_ = table.tm_ * roty * rotz;

    debug.DrawBox(table.tm_.GetColRef(3), table.bounding_box_.half_sizes_, color::LightYellow);
  }

  static void UpdateTennis(cfg::Models& models, CameraEul& cam, MainInput& input, DebugDraw& debug, float dt)
  {
    static ModelInstance& ball_instance = *models["model_ball"];
    static ModelInstance& table_instance = *models["model_table"];

    static Ball ball {ball_instance.tm_};
    static Table table {table_instance.tm_};

    static bool ball_inited = ResetBall(ball);

    UpdateTable(table, input, debug, dt);
    UpdateBall(ball, table, debug, dt);

    debug.DrawBasis(ball.tm_, 1.f);
    debug.DrawBasis(table.tm_, 1.f);
  }
}
