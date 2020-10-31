// *************************************************************
// File:    gl_pointers.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "gl_pointers.h"

PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
PFNGLBUFFERDATAPROC glBufferData = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
PFNGLCREATESHADERPROC glCreateShader = nullptr;
PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
PFNGLATTACHSHADERPROC glAttachShader = nullptr;
PFNGLDELETESHADERPROC glDeleteShader = nullptr;
PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = nullptr;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = nullptr;
PFNGLUNIFORM1IPROC glUniform1i = nullptr;
PFNGLUNIFORM1FPROC glUniform1f = nullptr;
PFNGLUNIFORM2FVPROC glUniform2fv = nullptr;
PFNGLUNIFORM3FVPROC glUniform3fv = nullptr;
PFNGLUNIFORM4FVPROC glUniform4fv = nullptr;
PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback = nullptr;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap = nullptr;

// PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert = nullptr;
// PFNGLGETDEBUGMESSAGELOGPROC glGetDebugMessageLog = nullptr;

namespace gdm {

namespace gl_pointers {

  bool GatherOpenglFunctions_v43()
  {
    glGenVertexArrays =
      (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glGenVertexArrays");
    glBindVertexArray =
      (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte*)"glBindVertexArray");
    glGenBuffers =
      (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
    glBindBuffer =
      (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
    glBufferData =
      (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
    glEnableVertexAttribArray = 
      (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress(
        (const GLubyte*)"glEnableVertexAttribArray");
    glDisableVertexAttribArray = 
      (PFNGLDISABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress(
        (const GLubyte*)"glDisableVertexAttribArray");
    glVertexAttribPointer = 
      (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress(
        (const GLubyte*)"glVertexAttribPointer");
    glCreateShader =
      (PFNGLCREATESHADERPROC)glXGetProcAddress((const GLubyte*)"glCreateShader");
    glShaderSource =
      (PFNGLSHADERSOURCEPROC)glXGetProcAddress((const GLubyte*)"glShaderSource");
    glCompileShader =
      (PFNGLCOMPILESHADERPROC)glXGetProcAddress((const GLubyte*)"glCompileShader");
    glCreateProgram =
      (PFNGLCREATEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glCreateProgram");
    glAttachShader =
      (PFNGLATTACHSHADERPROC)glXGetProcAddress((const GLubyte*)"glAttachShader");
    glDeleteShader =
      (PFNGLDELETESHADERPROC)glXGetProcAddress((const GLubyte*)"glDeleteShader");
    glLinkProgram =
      (PFNGLLINKPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glLinkProgram");
    glUseProgram =
      (PFNGLUSEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glUseProgram");
    glGetUniformLocation =
      (PFNGLGETUNIFORMLOCATIONPROC)glXGetProcAddress((const GLubyte*)"glGetUniformLocation");
    glUniformMatrix3fv =
      (PFNGLUNIFORMMATRIX3FVPROC)glXGetProcAddress((const GLubyte*)"glUniformMatrix3fv");
    glUniformMatrix4fv =
      (PFNGLUNIFORMMATRIX4FVPROC)glXGetProcAddress((const GLubyte*)"glUniformMatrix4fv");
    glUniform1i =
      (PFNGLUNIFORM1IPROC)glXGetProcAddress((const GLubyte*)"glUniform1i");
    glUniform1f =
      (PFNGLUNIFORM1FPROC)glXGetProcAddress((const GLubyte*)"glUniform1f");
    glUniform2fv =
      (PFNGLUNIFORM2FVPROC)glXGetProcAddress((const GLubyte*)"glUniform2fv");
    glUniform3fv =
      (PFNGLUNIFORM3FVPROC)glXGetProcAddress((const GLubyte*)"glUniform3fv");
    glUniform4fv =
      (PFNGLUNIFORM4FVPROC)glXGetProcAddress((const GLubyte*)"glUniform4fv");
    glGetProgramiv =
      (PFNGLGETPROGRAMIVPROC)glXGetProcAddress((const GLubyte*)"glGetProgramiv");
    glGetShaderiv =
      (PFNGLGETSHADERIVPROC)glXGetProcAddress((const GLubyte*)"glGetShaderiv");
    glGetShaderInfoLog =
      (PFNGLGETSHADERINFOLOGPROC)glXGetProcAddress((const GLubyte*)"glGetShaderInfoLog");
    glDebugMessageCallback =
      (PFNGLDEBUGMESSAGECALLBACKPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageCallback");
    glGenerateMipmap =
      (PFNGLGENERATEMIPMAPPROC)glXGetProcAddress((const GLubyte*)"glGenerateMipmap");

    // glDebugMessageInsert =
    //   (PFNGLDEBUGMESSAGEINSERTPROC)glXGetProcAddress((const GLubyte*)"glDebugMessageInsert");
    // glGetDebugMessageLog =
    //   (PFNGLGETDEBUGMESSAGELOGPROC)glXGetProcAddress((const GLubyte*)"glGetDebugMessageLog");

    return true;
  }

} // namespace gl_pointers

} // namespace gdm
