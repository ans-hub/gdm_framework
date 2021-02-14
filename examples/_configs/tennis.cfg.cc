// *************************************************************
// File:    config_dispatcher.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <string>
#include <vector>
#include <unordered_map>

#include <data/model_factory.h>
#include <render/camera_eul.h>
#include <window/main_input.h>
#include <math/vector3.h>
#include <math/general.h>
#include <math/bounding_box.h>
#include <math/sphere.h>
#include <system/diff_utils.h>

// Refs:
// Gabor Szauer - Game physics cookbook
// https://www.researchgate.net/figure/Collision-and-reflection-of-a-sphere-with-a-plane-locating-in-an-angle-th-with-the_fig16_319153891/download
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.104.4264&rep=rep1&type=pdf
// https://physics.stackexchange.com/questions/356674/angle-of-reflection-of-an-object-colliding-on-a-moving-wall
// https://www.khanacademy.org/science/physics/linear-momentum/momentum-tutorial/a/what-are-two-dimensional-collisions
// https://gamedev.stackexchange.com/questions/112299/balls-velocity-vector-reflect-against-a-point

//--private

namespace gdm::_private
{
  static const Vec3f v_g (0.f, -2.f, 0.f);
  static const float v_ball_max_y = -3.f; 

  struct OBB
  {
  	Vec3f world_pos_;
  	Vec3f half_size_;
  	const Mat4f* tm_;
  };

  struct Sphere
  {
    float radius_;
  	const Mat4f* tm_;
  };

  struct Table
  {
    Table(Mat4f& tm) : tm_{tm}, size_{1.f}
    {
      bounding_box_.half_size_.x = size_/2;
      bounding_box_.half_size_.y = 0.001f;
      bounding_box_.half_size_.z = size_/2;
      bounding_box_.tm_ = &tm_;
    }

    const float linear_speed_ = 10.f;
    const float angle_speed_ = 45.f;
    Mat4f& tm_;
    float size_;
    OBB bounding_box_;
  };

  struct Ball
  {
    Ball(Mat4f& tm) : tm_{tm}, vel_(0.f, -linear_speed_, 0.f)
    {
      bounding_sphere_.radius_ = 1.f;
      bounding_sphere_.tm_ = &tm_;
    }

    const float linear_speed_ = 1.f;
    Mat4f& tm_;
    Vec3f vel_;
    Vec3f accel_;
    Sphere bounding_sphere_;
  };

  Vec3f ClosestPoint(const OBB& obb, const Vec3f& point)
  {
  	Vec3f result = obb.tm_->GetCol(3);
  	Vec3f dir = point - result;

  	for (int i = 0; i < 3; ++i)
    {
      Vec3f axis = vec3::Normalize(obb.tm_->GetCol(i));
  		float distance = vec3::DotProduct(dir, axis);

  		if (distance > obb.half_size_[i])
  			distance = obb.half_size_[i];
  		if (distance < -obb.half_size_[i])
  			distance = -obb.half_size_[i];

  		result = result + (axis * distance);
  	}

  	return result;
  }

  bool SphereOOBIntersection(const Sphere& sphere, const OBB& obb)
  {
    Vec3f sphere_world_pos = sphere.tm_->GetCol(3);
    Vec3f closest_point = ClosestPoint(obb, sphere_world_pos);

    float dist_sq = vec3::SqLength(sphere_world_pos - closest_point);
    float radius_sq = sphere.radius_ * sphere.radius_;

    return dist_sq < radius_sq;
  }

  static bool IsCollided(Ball& ball, Table& table)
  {
    return SphereOOBIntersection(ball.bounding_sphere_, table.bounding_box_);
  }

  static void UpdateTable(Table& table, MainInput& input, float dt)
  {
    Vec3f fwd = table.tm_.GetCol(2);
    float vel_sign = 0.f;
    
    if (input.IsKeyboardBtnHold(DIK_UP))
      vel_sign = 1.f;
    else if (input.IsKeyboardBtnHold(DIK_DOWN))
      vel_sign = -1.f;
    
    Vec3f vel = vec3::Normalize(fwd) * table.linear_speed_ * dt * vel_sign;
    Vec3f pos = table.tm_.GetCol(3);
    pos += vel;
    table.tm_.SetCol(3, pos);

    float angle_y = 0.f;
    float angle_z = 0.f;

    if (input.IsKeyboardBtnHold(DIK_LEFT))
      if (input.IsKeyboardBtnHold(DIK_LSHIFT))
        angle_z = -1.f * table.angle_speed_ * dt;
      else
        angle_y = -1.f * table.angle_speed_ * dt;
    else if (input.IsKeyboardBtnHold(DIK_RIGHT))
      if (input.IsKeyboardBtnHold(DIK_LSHIFT))
        angle_z = 1.f * table.angle_speed_ * dt;
      else
        angle_y = 1.f * table.angle_speed_ * dt;

    Mat4f roty (1.f);
    Mat4f rotz (1.f);
    
    if (std::abs(angle_y) > math::kEpsilon)
      roty = matrix::MakeRotateY(angle_y);
    if (std::abs(angle_z) > math::kEpsilon)
      rotz = matrix::MakeRotateZ(angle_z);

    table.tm_ = table.tm_ * roty * rotz;
  }

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

  static void UpdateBall(Ball& ball, Table& table, float dt)
  {
    if (dt < math::kEpsilon)
      return;

    if (ball.tm_.GetCol(3).y < v_ball_max_y)
      ResetBall(ball);

    if (IsCollided(ball, table))
    {
      // r=d−2(d⋅n)n
      // v1' = v1 - 2(a1 - a2) * n
      // a1 = v1 . n and a2 = v2 . n.
      // a2 = 0
      Vec3f n = table.tm_.GetCol(1);
      Vec3f v1 = ball.vel_;
      Vec3f new_vel = v1 - (n * (2.f * (v1 * n)));

      ball.vel_ = new_vel;
    }

    Vec3f curr_vel = ball.vel_ * dt;
    ball.vel_ += ball.accel_ * dt;
    Vec3f pos = ball.tm_.GetCol(3);
    pos += curr_vel;
    ball.tm_.SetCol(3, pos);
  }

  static void UpdateTennis(std::unordered_map<std::string, ModelInstance*>& models, CameraEul& cam, MainInput& input, float dt)
  {
    static ModelInstance& ball_instance = *models["model_ball"];
    static ModelInstance& table_instance = *models["model_table"];

    static Ball ball {ball_instance.tm_};
    static Table table {table_instance.tm_};

    static bool ball_inited = ResetBall(ball);

    UpdateTable(table, input, dt);
    UpdateBall(ball, table, dt);
  }
}
