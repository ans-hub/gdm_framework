// *************************************************************
// File:    aliases.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <vector>
#include <memory>
#include <utility>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>

#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#ifndef GDM_WIN_ALIASES_H
#define GDM_WIN_ALIASES_H

namespace gdm {

typedef GLXContext (*glXCreateContextAttribsARBProc)(
  Display*, 
  GLXFBConfig, 
  GLXContext, 
  Bool, 
  const int*
);

auto xrr_deleter = [](XRRScreenConfiguration* x) { XRRFreeScreenConfigInfo(x); };
template<class X> auto x_deleter = [](X* x) { XFree(x); };

using XRRScreenPtr = std::unique_ptr<XRRScreenConfiguration, decltype(xrr_deleter)>;
using VisualPtr = std::unique_ptr<XVisualInfo, decltype(x_deleter<XVisualInfo>)>;

using ulong = unsigned long;
using WAttribs = XWindowAttributes;
using SWAttribs = XSetWindowAttributes;
using FBAttrs = std::vector<int>;
using VIAttrs = std::vector<int>;
using CTXAttrs = std::vector<int>;
using VModes = std::vector<std::pair<int,int>>;
using FBConfigs = std::vector<GLXFBConfig*>;

} // namespace gdm

#endif  // GDM_WIN_ALIASES_H