// *************************************************************
// File:    gl_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GL_UTILS_H
#define AH_GL_UTILS_H

#include <GL/gl.h>

namespace gdm {

namespace gl_utils {

  void EnableDebug();
  void GLAPIENTRY cb_debug_message(GLenum source, GLenum type, GLuint id, 
                                   GLenum severity, GLsizei length,
                                   const GLchar* message, const void* userParam);

} // namespace gl_utils

} // namespace gdm

#endif // AH_GL_UTILS_H