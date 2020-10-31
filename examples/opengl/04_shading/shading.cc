// *************************************************************
// File:    shading.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#define GM_MATRIX_INV_TRANSPOSE_TYPE 1

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glu.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <render/gl_pointers.h>
#include <render/gl_utils.h>
#include <render/texture.h>
#include <render/shader.h>
#include <render/camera_eul.h>
#include <render/mesh.h>
#include <render/debug_draw.h>
#include <render/fx_colors.h>

#include <libs/window/gl_window.h>
#include <data/ply_loader.h>
#include <data/helpers.h>
#include <system/files.h>
#include <system/timer.h>
#include <system/array_utils.h>
#include <system/string_utils.h>

#include <math/math.h>
#include <math/vector2.h>
#include <math/vector3.h>
#include <math/matrix.h>

using namespace gdm;

#pragma GCC push_options
#pragma GCC optimize("O0")

const bool gDebugDraw {false};
const float gRotLampSpeed {0.5f};

int main(int argc, const char** argv)
{
  const int frames_per_sec {60};
  Timer timer {frames_per_sec};
  GlWindow win = gl_window::MakeCentered(800, 600, "Advanced example");
  
  gl_pointers::GatherOpenglFunctions_v43();
  gl_utils::EnableDebug();
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_FRAMEBUFFER_SRGB);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LEQUAL);
  glDepthRange(0.0f, 1.0f);

  DebugDraw debug {};

  ply::Loader ply {argv[1]};
  Mesh obj(ply);
 
  // Uncomment it to test normal mapping works correctly with different model's pos and rots
 
  // Mat4f rot = matrix::MakeRotate(6.f, vec3::Normalize(Vec3f{-0.5f, 0.2f, 0.5f}));
  // obj.SetTm(rot * Mat4f(1.f));
  // obj.SetPos(Vec3f(3.f, -2.f, -2.f));

  std::string spec_fname  = ply.GetString("specular_map");
  std::string spec_path   = file::GetPathFromFullPath(argv[1]) + spec_fname;
  AbstractImage spc = helpers::MakeImage(spec_path.c_str());
  Texture2d specular_map(spc, false);

  std::string img_fname  = ply.GetString("texture"); // todo: rename to diffuse_map
  std::string img_path   = file::GetPathFromFullPath(argv[1]) + img_fname;
  AbstractImage img = helpers::MakeImage(img_path.c_str());
  Texture2d texture(img, true);

  std::string map_fname  = ply.GetString("normal_map");
  std::string map_path   = file::GetPathFromFullPath(argv[1]) + map_fname;
  AbstractImage map = helpers::MakeImage(map_path.c_str());
  Texture2d normal_map(map, false);

  ply = ply::Loader {"../00_models_old/sphere.ply"};
  Mesh lamp(ply);
  lamp.SetPos(Vec3f{0.f, -0.3f, 5.f});
  const Vec3f lamp_color{1.f, 1.f, 1.f};
  Vec3f lamp_dir{-0.2f, 0.2f, 1.f};
  lamp_dir.Normalize();

  Shader phong_shader("phong.vert", "phong.frag");
  if (!map_fname.empty())
    phong_shader = Shader("nmap.vert", "nmap.frag");
  Shader lamp_shader("flat.vert", "flat.frag");  
  Shader debug_shader("debug.vert", "debug.frag");  

  CameraEul cam (75.f, 4.f/3.f, 0.1f, 100.f);
  cam.SetPos({0.f, 3.f, 5.f});

  do {
    timer.Start();
    win.Clear();
    cam.ProcessInput(win, timer.GetLastDt());

    Mat4f model = obj.GetTm();
    Mat4f view = cam.GetViewMx();
    Mat4f proj = cam.GetProjectionMx();
    Mat4f mvp = proj * view * model;

    Mat4f mx_roty = matrix::MakeRotateY(gRotLampSpeed);
    lamp.SetPos(mx_roty * lamp.GetPos()); // todo
    Mat4f lamp_orient = mx_roty * Mat4f(1.f);
    lamp_orient.SetCol(3, lamp.GetPos());
    lamp.SetTm(lamp_orient);
    lamp_dir = vec3::Normalize(obj.GetPos() - lamp.GetPos());

    if (gDebugDraw)
    {
      Mat4f debug_mvp = proj * view * Mat4f(1.f);
      debug_shader.Use();
      debug_shader.SetMat<4>("mx_mvp", matrix::ValuePtr(debug_mvp));
      debug.AddLine(lamp.GetPos(), obj.GetPos(), color::Red);
      debug.AddDirs(obj.GetVxsCoords(), obj.GetVxsTangents(), 0.2f, color::Blue);
      debug.AddDirs(obj.GetVxsCoords(), obj.GetVxsNormals(), 0.3f, color::Red);
      debug.Draw();
    }
    
    phong_shader.Use();
    phong_shader.SetMat<4>("mx_view", matrix::ValuePtr(view));
    phong_shader.SetMat<4>("mx_model", matrix::ValuePtr(model));
    phong_shader.SetMat<4>("mx_mvp", matrix::ValuePtr(mvp));
    phong_shader.SetVec<3>("view_pos", vec3::ValuePtr(cam.GetTm().GetCol(3)));
    phong_shader.SetVec<3>("light_col", vec3::ValuePtr(lamp_color));
    phong_shader.SetVec<3>("light_pos", vec3::ValuePtr(lamp.GetPos()));
    phong_shader.SetVal<int>("map_sampler", 0);
    phong_shader.SetVal<int>("tex_sampler", 1);
    phong_shader.SetVal<int>("spec_sampler", 2);
    normal_map.Activate(0);
    texture.Activate(1);
    specular_map.Activate(2);
    obj.Draw();

    Mat4f lamp_mvp = proj * view * lamp.GetTm();
    lamp_shader.Use();
    lamp_shader.SetMat<4>("mx_mvp", matrix::ValuePtr(lamp_mvp));
    lamp_shader.SetVec<3>("color", vec3::ValuePtr(lamp_color));
    lamp.Draw();

    win.Render();
    timer.Wait();
    timer.End();
  } while (!win.IsClosed());

  return 0;
}

#pragma GCC pop_options
