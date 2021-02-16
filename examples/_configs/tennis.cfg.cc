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
  static const Vec3f v_g (0.f, -3.f, 0.f);
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
    Table(Mat4f& tm)
      : tm_{tm}
      , size_{matrix::DecomposeToScale(tm).x}
    {
      bounding_box_.half_size_.x = size_/2.f;
      bounding_box_.half_size_.y = size_/2.f;
      bounding_box_.half_size_.z = size_/2.f;
      bounding_box_.tm_ = &tm_;
    }

    const float linear_speed_ = 4.f;
    const float angle_speed_ = 45.f;
    Mat4f& tm_;
    Vec3f vel_;
    float size_;
    OBB bounding_box_;
  };

  struct Ball
  {
    Ball(Mat4f& tm)
      : tm_{tm}
      , vel_(0.f, -linear_speed_, 0.f)
      , size_{matrix::DecomposeToScale(tm).x}
    {
      bounding_sphere_.radius_ = size_/2.f;
      bounding_sphere_.tm_ = &tm_;
    }

    const float linear_speed_ = 1.f;
    Mat4f& tm_;
    float size_;
    Vec3f vel_;
    Vec3f accel_;
    Sphere bounding_sphere_;
  };

  struct ColResource
  {
    Vec3f normal;
    float penetration;
    Vec3f contact_point;
  };

  Vec3f ClosestPoint(const OBB& obb, const Vec3f& point, Vec3f& collision_norm)
  {
  	Vec3f result = obb.tm_->GetCol(3);
  	Vec3f dir = point - result;

  	for (int i = 0; i < 3; ++i)
    {
      Vec3f axis = vec3::Normalize(obb.tm_->GetCol(i));
  		float distance = vec3::DotProduct(dir, axis);

  		if (distance > obb.half_size_[i])
      {
        collision_norm += axis;
  			distance = obb.half_size_[i];
      }
      else if (distance < -obb.half_size_[i])
      {
        collision_norm += -axis;
  			distance = -obb.half_size_[i];
      }
      collision_norm.Normalize();
  		result = result + (axis * distance);
  	}

  	return result;
  }

  bool SphereOOBIntersection(const Sphere& sphere, const OBB& obb, ColResource& col)
  {
    Vec3f sphere_world_pos = sphere.tm_->GetCol(3);
    Vec3f closest_point = ClosestPoint(obb, sphere_world_pos, col.normal);

    float dist_sq = vec3::SqLength(sphere_world_pos - closest_point);
    float radius_sq = sphere.radius_ * sphere.radius_;

    if (dist_sq < radius_sq)
    {
      float dist = vec3::Length(sphere_world_pos - closest_point) - sphere.radius_;
      col.penetration = dist / 2;
      col.contact_point = closest_point;
      return true;
    }
    return false;
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

    ColResource col {};

    if (SphereOOBIntersection(ball.bounding_sphere_, table.bounding_box_, col))
    {
      Vec3f pen_dir = vec3::Normalize(col.contact_point - ball.tm_.GetCol(3));
      Vec3f corr_pos = ball.tm_.GetCol(3);
      corr_pos += (pen_dir * col.penetration * 2);
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
  }

  static void UpdateTable(Table& table, MainInput& input, float dt)
  {
    Vec3f fwd = table.tm_.GetCol(2);
    float vel_sign = 0.f;
    
    if (input.IsKeyboardBtnHold(DIK_UP))
      vel_sign = 1.f;
    else if (input.IsKeyboardBtnHold(DIK_DOWN))
      vel_sign = -1.f;
    
    table.vel_ = vec3::Normalize(fwd) * table.linear_speed_ * dt * vel_sign;
    Vec3f pos = table.tm_.GetCol(3);
    pos += table.vel_;
    table.tm_.SetCol(3, pos);

    float angle_y = 0.f;
    float angle_z = 0.f;

    if (input.IsKeyboardBtnHold(DIK_LEFT))
    {
      if (input.IsKeyboardBtnHold(DIK_LSHIFT))
        angle_z = -1.f * table.angle_speed_ * dt;
      else
        angle_y = -1.f * table.angle_speed_ * dt;
    }
    else if (input.IsKeyboardBtnHold(DIK_RIGHT))
    {
      if (input.IsKeyboardBtnHold(DIK_LSHIFT))
        angle_z = 1.f * table.angle_speed_ * dt;
      else
        angle_y = 1.f * table.angle_speed_ * dt;
    }

    Mat4f roty (1.f);
    Mat4f rotz (1.f);
    
    if (std::abs(angle_y) > math::kEpsilon)
      roty = matrix::MakeRotateY(angle_y);
    if (std::abs(angle_z) > math::kEpsilon)
      rotz = matrix::MakeRotateZ(angle_z);

    table.tm_ = table.tm_ * roty * rotz;
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
