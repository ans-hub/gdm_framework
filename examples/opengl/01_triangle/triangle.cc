// *************************************************************
// File:    triangle.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <iostream>
#include <chrono>
#include <string>
#include <cstdio>
#include <cassert>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glu.h>

#include <libs/window/enums.h>
#include <libs/window/helpers.h>
#include <libs/window/gl_window.h>
#include <libs/window/gl_pointers.h>
#include <libs/window/exceptions.h>

using namespace gdm;

FBAttrs get_fb_attrs() 
{
  return {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, true,
    GLX_RED_SIZE, 1,
    GLX_GREEN_SIZE, 1,
    GLX_BLUE_SIZE, 1,
    None
  };
}

CTXAttrs get_ct_attrs()
{
  return {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    None
  };
}

static const char* vertex_shader = 
  "#version 400\n"
  "layout(location=0) in vec3 in_pos;"
  "layout(location=1) in vec4 in_col;"
  "out vec4 ex_col;"
  "void main() {"
  "  gl_Position = vec4(in_pos,1.f);"
  "  ex_col = in_col;"
  "}";

static const char* fragment_shader = 
  "#version 400\n"
  "in vec4 ex_col;"
  "out vec4 frag_col;"
  "void main() {"
  "  frag_col = ex_col;"
  "}";

void wait(int fps)
{
  timespec ts;
  ts.tv_sec  = fps / 1000;
  ts.tv_nsec = fps * 1000000;
  while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) { }
}

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, 
                                GLenum severity, GLsizei length,
                                const GLchar* message, const void* userParam)
{
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
         (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type, severity, message);
}

// http://openglbook.com/chapter-2-vertices-and-shapes.html

int main()
{
  Size sz {800, 600};
  Pos pos  = helpers::GetXYToMiddle(sz.w, sz.h); 
  GlWindow win (get_fb_attrs(), get_ct_attrs(), pos, sz, "Modern ctx");
  gl_pointers::GatherOpenglFunctions_v43();

  static const GLfloat vertices[] = {
    -0.5f, -0.5f, 0.f,
     0.f,  0.5f, 0.f,
     0.5f, -0.5f, 0.f
  };

  static const GLfloat colors[] = {
    1.f, 0.f, 0.f, 1.f,
    0.f, 1.f, 0.f, 1.f,
    0.f, 0.f, 1.f, 1.f
  };

  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback, 0);

  // VAO - Vertex attribute object, a way to say opengl how does data stored
  // in VBO (i.e. one vertex consists of point, color & tex coordinate)

  GLuint vao {0};
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // VBO - Vertex buffer object, is just a way to recieve data to gpu memory
  // Here we request given number of free VBO ids, then bind this to current
  // buffer given id and put there data

  GLuint vbo {0};
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);  // 3 sz of vertex line
  glEnableVertexAttribArray(0);

  GLuint colorBuffer {0};
  glGenBuffers(1, &colorBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);  // 4 sz of color line
  glEnableVertexAttribArray(1);

  // Shader creating and compiling routines

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  
  int status;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
  {
    char info[256];
    glGetShaderInfoLog(vs, 256, nullptr, info);
    std::cerr << "Vertex shader compilation error\n" << info << '\n';
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  
  glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
  if (status != GL_TRUE)
  {
    char info[256];
    glGetShaderInfoLog(fs, 256, nullptr, info);
    std::cerr << "Fragment shader compilation error\n" << info << '\n';
  }

  GLuint prog = glCreateProgram();
  glAttachShader(prog, fs);
  glAttachShader(prog, vs);
  glLinkProgram(prog);
  glGetProgramiv(prog, GL_LINK_STATUS, &status);
  assert(status == GL_TRUE && "Shader linking failed");

  glUseProgram(prog);

  const int k_first_active_vao {0};
  const int k_vxs_cnt {3};

  do {
    win.Clear();
    glDrawArrays(GL_TRIANGLES, k_first_active_vao, k_vxs_cnt);
    win.Render();
    wait(60);
  } while (!win.IsClosed());

  return 0;
}
