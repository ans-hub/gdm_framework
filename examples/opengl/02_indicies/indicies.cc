// *************************************************************
// File:    indicies.cc
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
#include <system/stream_utils.h>
#include <data/ply_loader.h>

using namespace gdm;

#pragma GCC push_options
#pragma GCC optimize("O0")

struct Vertex
{
  float pos[3];
  float col[4];

}; // struct Vertex

int main(int argc, const char** argv)
{
  char fname[256] {};
  if (argc < 2)
    strcpy(fname, "cube.ply");
  else
    strcpy(fname, argv[1]);

  ply::Loader ply {};
  std::ifstream fss {fname};
  if (fss)
    ply.Load(fss);
  else
  {
    std::cerr << "Invalid ply file " << fname << '\n';
    return -1;
  }

  std::vector<float> vertices = ply::helpers::GetPlyData<float>(
    ply, "vertex", {"x", "y", "z", "r", "g", "b", "a"}, true);
  std::vector<uint> faces = ply::helpers::GetPlyData<uint>(
    ply, "face", {"vertex_indices"}, false);

  for (const auto& elem : vertices)
    std::cout << elem << ' ';
  std::cout << '\n'; 

  for (const auto& elem : faces)
    std::cout << elem << ' ';
  std::cout << '\n'; 

  const int frames_per_sec {60};
  Timer timer {frames_per_sec};
  GlWindow win = gl_utils::MakeCenteredWindow(800, 600, "Advanced example");

  gl_pointers::GatherOpenglFunctions_v43();
  gl_utils::EnableDebug();
  glEnable(GL_CULL_FACE);

  const size_t vertex_size = sizeof(Vertex);
  const size_t buffer_size = vertices.size() * sizeof(float);
  const size_t color_offset = sizeof(Vertex::pos);

  std::cerr << "vertex size: " << vertex_size << '\n'
            << "buffer size: " << buffer_size << '\n'
            << "color_offset: " << color_offset << '\n';

  GLuint vao {0};
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint vbo {0};
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, buffer_size, &vertices[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertex_size, 0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertex_size, (GLvoid*)color_offset);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  GLuint ibo {0};
  glGenBuffers(1, &ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint)*faces.size(), faces.data(), GL_STATIC_DRAW);

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

  float curr_angle = 0.f;
  const float angles_per_sec = 25.f;
  const float angles_per_frame = angles_per_sec / frames_per_sec;

  const float min_scale = 0.5f;
  const float max_scale = 1.5f;
  const float full_scale_time = 5.f;
  const float scale_per_frame = (max_scale - min_scale) / (frames_per_sec * full_scale_time);
  float curr_scale = min_scale;
  int curr_scale_sign = 1;

  do {
    win.Clear();
    
    curr_angle += angles_per_frame;
    glm::clamp(curr_angle, 0.f, 360.f);
    curr_scale += scale_per_frame * curr_scale_sign;
    if (curr_scale >= max_scale || curr_scale <= min_scale)
      curr_scale_sign *= -1;

    glm::mat4 proj = glm::perspective(glm::radians(75.f), 4.f/3.f, 0.1f, 100.f);
    glm::mat4 model = glm::rotate(glm::mat4(1), glm::radians(curr_angle), glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 view = glm::translate(glm::mat4(1), glm::vec3{0,-1.2f,-4}); // inverted value
    glm::mat4 res = proj * view * model;

    glUniformMatrix4fv(mx_id, 1, false, glm::value_ptr(res));
    glDrawElements(GL_TRIANGLES, faces.size(), GL_UNSIGNED_INT,
      (GLvoid*)(sizeof(uint) * 0));

    win.Render();
    timer.Wait();
  } while (!win.IsClosed());

  return 0;
}
#pragma GCC pop_options
