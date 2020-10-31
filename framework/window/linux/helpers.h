// *************************************************************
// File:    helpers.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GDM_WIN_HELPERS_H
#define GDM_WIN_HELPERS_H

#ifndef GLX_GLXEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#endif

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>   
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <GL/glu.h>

#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <limits>

#include "window/enums.h"
#include "window/aliases.h"
#include "window/exceptions.h"

namespace gdm::helpers {

  // Defaults

  FBAttrs GetDefaultFBAttribs();
  FBAttrs GetDefaultFBAttribs42();
  SWAttribs GetDefaultWinAttribs(Display*, ::Window, XVisualInfo*);
  VIAttrs GetDefaultVisualAttributes();
  CTXAttrs GetGlContextAttributes42();
  CTXAttrs GetDefaultGlContextAttributes();

  // Config routines

  FBConfigs GetFBConfigs(Display*, FBAttrs&);
  int GetBestMSAAVisual(Display*, FBConfigs&);
  VisualPtr ChooseGlVisual(Display*, GLXFBConfig&);
  VisualPtr ChooseGlVisual(Display*);
  bool FindFlagInFBAttrs(const FBAttrs&, int, int);

  // Context routines

  GLXContext CreateGlContext(Display*, GLXFBConfig&, CTXAttrs& attrs);
  GLXContext CreateGlContext(Display*, XVisualInfo*);
  GLXPbuffer CreatePBuffer(Display*, int w, int h, GLXFBConfig&);
  bool ToggleVerticalSync(Display*, ::Window drawable, bool status);

  // Windows routines

  ::Window CreateGlWindow(Display*, ::Window root, XVisualInfo*, SWAttribs&, int x, int y, int w, int h);
  ::Window CreateSmpWindow(Display*, ::Window root, int scr, int x, int y, int w, int h);
    
  // Video modes and dimensions routines

  VModes GetVideoModes(Display*, ::Window);
  int GetCurrentVideoMode(Display*, ::Window);
  int ChangeVideoMode(Display*, ::Window, int mode);
  void GetWindowDimension(Display*, ::Window, int* w, int* h);
  int FindNearestVideoMode(int w, int h);
  int FindVideoMode(int w, int h);

  // Other usefull stuff

  void ChangeWindowName(Display*, ::Window, const char* name);
  ::Window GetFocusedWindow(Display*);
  void HideCursor(Display*, ::Window);
  void UnhideCursor(Display*, ::Window);
  Pos GetXYToMiddle(int w, int h);
  
  // Gl extensions routines

  const char* GetAllGlExtensions();
  const char* GetAllGlxExtensions(Display*);
  bool IsExtensionSupported(const char* extension);
  bool IsExtensionSuppored(const char* extension_list, const char* extension);
  
  // Debug stuff 

  auto GetGlContextVersion() -> std::pair<int,int>;
  auto GetGlContextProfile() -> std::pair<int,int>;
  auto GetGlxVersion(Display*) -> std::pair<int,int>;
  void PrintGlInfo(std::ostream&);

  // Sending notifies to WM

  bool SendToggleFullscreenNotify(Display*, ::Window root, ::Window win);
  void SendCloseWindowNotify(Display*, ::Window root, ::Window win);
  void SendToggleOnTopNotify(Display*, ::Window root, ::Window win);
 
  namespace ptr {

    XRRScreenPtr XRRGetScreenInfo(Display*, ::Window);
    VisualPtr glXGetVisualFromFBConfig(Display*, GLXFBConfig&);
    VisualPtr glXChooseVisual(Display*, int scr, int* data);
    
  } // namespace ptr

} // namespace gdm::helpers

std::ostream& operator<<(std::ostream&, std::pair<int,int>&);

#endif  // GDM_WIN_HELPERS_H
