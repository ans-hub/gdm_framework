// *************************************************************
// File:    gl_utils.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gl_utils.h"

#include <stdio.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>
#include <GL/glu.h>

#include "gl_pointers.h"

namespace gdm {

void GLAPIENTRY gl_utils::cb_debug_message(
  GLenum /*source*/, GLenum type, GLuint /*id*/, GLenum severity,
  GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
  fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
         (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
          type, severity, message);
}

void gl_utils::EnableDebug()
{
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(cb_debug_message, 0);
}

} // namespace gdm
