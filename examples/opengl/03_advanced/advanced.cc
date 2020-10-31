// *************************************************************
// File:    advanced.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

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

#include "cases/gl_utils.h"

#include <libs/window/enums.h>
#include <libs/window/helpers.h>
#include <libs/window/gl_window.h>
#include <libs/window/gl_pointers.h>
#include <libs/window/exceptions.h>
#include <system/files.h>
#include <system/timer.h>
#include <system/array_utils.h>
#include <math/normal_utils.h>
#include <data/ply_loader.h>

using namespace gdm;

#pragma GCC push_options
#pragma GCC optimize("O0")

struct Vertex
{
  float pos[3];
  float col[3];
  float norm[3];

}; // struct Vertex

template<class T>
void debug_print(const std::vector<T>& /*arr*/, std::ostream& /*oss*/) { }

template<>
void debug_print<glm::vec3>(const std::vector<glm::vec3>& arr, std::ostream& oss)
{
  for (const auto& elem : arr)
    oss << elem.x << ' ' << elem.y << ' ' << elem.z << ' ';
  oss << '\n'; 
}

template<>
void debug_print<uint>(const std::vector<uint>& arr, std::ostream& oss)
{
  for (const auto& elem : arr)
    oss << elem << ' ';
  oss << '\n'; 
}

ply::Loader try_load_model(int argc, const char** argv)
{
  char fname[256] {};
  if (argc < 2)
    strcpy(fname, "cube.ply");
  else
    strcpy(fname, argv[1]);

  ply::Loader ply {};
  std::ifstream fss {fname};
  assert(fss && "Invalid ply file");
  ply.Load(fss);
  return ply;
}

int main(int argc, const char** argv)
{
  ply::Loader ply = try_load_model(argc, argv);
  std::vector<glm::vec3> coords = ply::helpers::LoadXYZ<glm::vec3>(ply);
  std::vector<glm::vec3> colors = ply::helpers::LoadRGB<glm::vec3>(ply);
  std::vector<glm::uvec3> faces = ply::helpers::LoadFaces<glm::uvec3>(ply);
  arr_utils::normalize(&coords[0].x, coords.size() * 3, -1.f, 1.f);
  std::vector<glm::vec3> normals = math_utils::ComputeVxsNormals(coords, faces);
  assert(normals.size() == coords.size());

  debug_print(coords, std::cerr);
  debug_print(colors, std::cerr);
  debug_print(faces, std::cerr);

  std::vector<float> vxs {};
  vxs = arr_utils::expand(&coords[0].x, 3, coords.size(), &colors[0].x, 3, colors.size());
  vxs = arr_utils::expand(&vxs[0], 6, vxs.size() / 6, &normals[0].x, 3, normals.size());

  const int frames_per_sec {60};
  Timer timer {frames_per_sec};
  GlWindow win = gl_utils::MakeCenteredWindow(800, 600, "Advanced example");
  
  gl_pointers::GatherOpenglFunctions_v43();
  gl_utils::EnableDebug();
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  const size_t vertex_size = sizeof(Vertex);
  const size_t buffer_size = vxs.size() * vertex_size;
  const size_t color_offset = sizeof(Vertex::pos);
  const size_t normal_offset = color_offset + sizeof(Vertex::col);

  std::cerr << "vertex size: " << vertex_size << '\n'
            << "buffer size: " << buffer_size << '\n'
            << "color_offset: " << color_offset << '\n'
            << "normal_offset: " << normal_offset << '\n';

  GLuint vao {0};
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo {0};
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, &vxs[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*)color_offset);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*)normal_offset);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  GLuint ibo {0};
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*faces.size() * 3, faces.data(), GL_STATIC_DRAW);

  GLuint prog = glCreateProgram();

  GLuint vert_shader = gl_utils::GetShader(GL_VERTEX_SHADER, "simple.vert");
  if (!gl_utils::IsShaderCompiled(vert_shader))
    return gl_utils::GetShaderInfoLog(vert_shader, "simple.vert", std::cerr);
  glAttachShader(prog, vert_shader);

  GLuint frag_shader = gl_utils::GetShader(GL_FRAGMENT_SHADER, "simple.frag");
  if (!gl_utils::IsShaderCompiled(frag_shader))
    return gl_utils::GetShaderInfoLog(frag_shader, "simples.frag", std::cerr);
  glAttachShader(prog, frag_shader);
  
  glLinkProgram(prog);
  assert(gl_utils::IsProgramLinked(prog) && "Shader program linking fail");
  glUseProgram(prog);

  GLuint mx_id = glGetUniformLocation(prog, "mvp");

  const float angles_per_sec = 25.f;
  const float angles_per_frame = angles_per_sec / frames_per_sec;
  const float min_scale = 0.5f;
  const float max_scale = 1.5f;
  const float full_scale_time = 5.f;
  const float scale_per_frame = (max_scale - min_scale) / (frames_per_sec * full_scale_time);

  int curr_scale_sign = 1;
  float curr_scale = min_scale;
  float curr_angle = 0.f;

  do {
    win.Clear();
    
    curr_angle += angles_per_frame;
    glm::clamp(curr_angle, 0.f, 360.f);
    curr_scale += scale_per_frame * curr_scale_sign;
    if (curr_scale >= max_scale || curr_scale <= min_scale)
      curr_scale_sign *= -1;

    glm::mat4 proj = glm::perspective(glm::radians(75.f), 4.f/3.f, 0.1f, 100.f);
    glm::mat4 model = glm::rotate(glm::mat4(1), glm::radians(curr_angle), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 view = glm::translate(glm::mat4(1), glm::vec3{0.f,2.f,-6.f}); // inverted value
    glm::mat4 res = proj * view * model;

    glUniformMatrix4fv(mx_id, 1, false, glm::value_ptr(res));
    glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT,
      (GLvoid*)(sizeof(uint) * 0));

    win.Render();
    timer.Wait();
  } while (!win.IsClosed());

  return 0;
}
#pragma GCC pop_options
