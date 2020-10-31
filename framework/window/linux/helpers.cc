// *************************************************************
// File:    helpers.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "helpers.h"

// This is default gl framebuffer attributes represented in value/pair
// and last is None. Used in glXChooseFbConfig

FBAttrs gdm::helpers::GetDefaultFBAttribs()
{
  // MSAA is not supported for window drawable (only pbuffer of pixmap)
  // and when not fs. thus comment sample buffers and comment GetBestMatchesVI
  
  return {
    GLX_CONFIG_CAVEAT   , GLX_NONE,
    GLX_X_RENDERABLE    , True,
    GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT | GLX_PBUFFER_BIT,
    GLX_RENDER_TYPE     , GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
    GLX_RED_SIZE        , 8,
    GLX_GREEN_SIZE      , 8,
    GLX_BLUE_SIZE       , 8,
    GLX_ALPHA_SIZE      , 8,
    GLX_DEPTH_SIZE      , 24,
    GLX_STENCIL_SIZE    , 8,
    GLX_BUFFER_SIZE     , 32,
    GLX_DOUBLEBUFFER    , True,
    // GLX_SAMPLE_BUFFERS  , 1,
    // GLX_SAMPLES         , 4,
    None
  };
}

FBAttrs gdm::helpers::GetDefaultFBAttribs42()
{
  return {
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_DOUBLEBUFFER, true,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    None
  };
}

SWAttribs gdm::helpers::GetDefaultWinAttribs(Display* disp, Window wnd, XVisualInfo* vi)
{
  XSetWindowAttributes attr;
  attr.colormap   = XCreateColormap(disp, wnd, vi->visual, AllocNone);
  attr.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask |
                    PointerMotionMask | VisibilityChangeMask;
  attr.override_redirect = true;
  attr.background_pixmap = None;
  attr.background_pixel = 0;
  attr.border_pixel = 0;

  return attr;
}

// GL attributes used in old-style glChooseGlVisual

VIAttrs gdm::helpers::GetDefaultVisualAttributes()
{
  return {
    GLX_RGBA,
    GLX_DEPTH_SIZE,
    24,
    GLX_DOUBLEBUFFER,
    None
  };
}

CTXAttrs gdm::helpers::GetGlContextAttributes42()
{
  return {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    None
  };
}

CTXAttrs gdm::helpers::GetDefaultGlContextAttributes()
{
  return {
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3, //4
    GLX_CONTEXT_MINOR_VERSION_ARB, 0, //1
  // GLX_CONTEXT_PROFILE_MASK_ARB,
  // GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    None
  };
}

// Returns available framebuffer configurations based on given fb
// attributes. More at:
//  https://www.khronos.org/opengl/wiki/Framebuffer
//  https://www.khronos.org/opengl/wiki/Default_Framebuffer

FBConfigs gdm::helpers::GetFBConfigs(Display* disp, FBAttrs& attrs)
{
  int cnt {0};
  GLXFBConfig* fbc = NULL;
  
  // Try to get any config (if something goes wrong) - see note below

  fbc = glXChooseFBConfig(disp, DefaultScreen(disp), attrs.data(), &cnt);
  if (!fbc)
    fbc = glXChooseFBConfig(disp, DefaultScreen(disp), NULL, &cnt);

  if (!fbc)
    throw WinException("No appropriate fbconfigs found", errno);    

  FBConfigs result {};
  for (int i = 0; i < cnt; ++i)
  {
    GLXFBConfig* curr = &fbc[i];
    result.push_back(curr);
  } 
  return result;

  // Note: on the cygwin glXChooseFBConfig() gives NULL when requested
  // attr GLX_CONFIG_CAVEAT = GLX_NONE
}

// Choose framebuffer config with max sample buffers 

int gdm::helpers::GetBestMSAAVisual(Display* disp, FBConfigs& cfg)
{
  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

  for (std::size_t i = 0; i < cfg.size(); ++i)
  {
    auto vi = ptr::glXGetVisualFromFBConfig(disp, *cfg[i]);
    if (vi.get()) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(disp, *cfg[i], GLX_SAMPLE_BUFFERS, &samp_buf );
      glXGetFBConfigAttrib(disp, *cfg[i], GLX_SAMPLES       , &samples  );
  
      if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) )
        best_fbc = i, best_num_samp = samples;
      if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
        worst_fbc = i, worst_num_samp = samples;
    }
  }
  return best_fbc;
}

// Choose gl visual capabilities from framebuffer config (modern way)

VisualPtr gdm::helpers::ChooseGlVisual(Display* disp, GLXFBConfig& cfg)
{
  auto vi = ptr::glXGetVisualFromFBConfig(disp, cfg);
  if (!vi)
    throw WinException("No appropriate visual found", errno);
  return vi;
}

// Choose gl visual (deprecated way)

VisualPtr gdm::helpers::ChooseGlVisual(Display* disp)
{
  auto attr = GetDefaultVisualAttributes();
  auto vi = ptr::glXChooseVisual(disp, 0, attr.data());
  if (!vi)
    throw WinException("No appropriate visual found", errno);
  return vi;
}

// Searches set bit in fbattrs array

bool gdm::helpers::FindFlagInFBAttrs(const FBAttrs& fb_attrs, int attr, int flag)
{
  for (auto& attr_item : fb_attrs)
    if ((attr_item == attr) && (attr_item & flag))
      return true;
  return false;
}

GLXContext gdm::helpers::CreateGlContext(Display* disp, GLXFBConfig& fb_cfg, CTXAttrs& attrs)
{ 
  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
  glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );

  GLXContext glc = 0;
  auto glxExts = GetAllGlxExtensions(disp);
  if (!IsExtensionSupported(glxExts, "GLX_ARB_create_context")){
    std::cerr << "GLX_ARB_create_context is not supported\n";
    glc = glXCreateNewContext(disp, fb_cfg, GLX_RGBA_TYPE, 0, True);
  }
  else
    glc = glXCreateContextAttribsARB(disp, fb_cfg, 0, true, attrs.data());
  XSync(disp, False);
  
  if (!glc)
    throw WinException("Cannot create gl context", errno);

  if (!glXIsDirect(disp, glc))
    throw WinException("Indirect context", errno);      
  return glc;
}

// Old style context creating

GLXContext gdm::helpers::CreateGlContext(Display* disp, XVisualInfo* vi)
{
  GLXContext glc = glXCreateContext(disp, vi, NULL, GL_TRUE);
  if (!glc)
    throw WinException("Cannot create gl context", errno);
  return glc;
}

GLXPbuffer gdm::helpers::CreatePBuffer(Display* disp, int w, int h, GLXFBConfig& cfg)
{
  int attrs[] = {
    GLX_PBUFFER_WIDTH, w,
    GLX_PBUFFER_HEIGHT, h,
    None
  };
  return glXCreatePbuffer(disp, cfg, attrs);
}

// https://www.khronos.org/opengl/wiki/Swap_Interval#In_Linux_.2F_GLX
// Note : this is the good example of how to get gl extensions
// Also we may change it by environment, example: $ vblank_mode=0 glxgears 

bool gdm::helpers::ToggleVerticalSync(Display* disp, ::Window drawable, bool status)
{
  using PtrExt = PFNGLXSWAPINTERVALEXTPROC;
  using PtrSgi = PFNGLXSWAPINTERVALSGIPROC;

  // First, we look into glxext.h

  PtrExt glXSwapIntervalEXT = NULL;
  PtrSgi glXSwapIntervalSGI = NULL;

  // The check if extensions presents

  auto exts = GetAllGlxExtensions(disp);
  if (IsExtensionSupported(exts, "GLX_EXT_swap_control"))
  {
    glXSwapIntervalEXT = (PtrExt)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
    glXSwapIntervalEXT(disp, static_cast<GLXDrawable>(drawable), static_cast<int>(status));
    return true;
  }
  else if (IsExtensionSupported(exts, "GLX_SGI_swap_control"))
  {
    glXSwapIntervalSGI = (PtrSgi)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");
    glXSwapIntervalSGI(static_cast<int>(status)); // todo: ops, interval shouldn`t be less than 1
    return true;
  }
  else
    return false;
}

::Window gdm::helpers::CreateGlWindow(Display* disp, ::Window root, XVisualInfo* vi, SWAttribs& attr, int x, int y, int w, int h)
{
  ::Window win = XCreateWindow(disp, root, x, y, w, h, 0, vi->depth,
                 InputOutput, vi->visual, CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask, &attr);
  if (!win)
    throw WinException("Cannot create gl window", errno);
  return win;
}

::Window gdm::helpers::CreateSmpWindow(Display* disp, ::Window root, int scr, int x, int y, int w, int h)
{
  ulong fg = BlackPixel(disp, scr);
  ulong bg = WhitePixel(disp, scr);
  int depth = 1;
  ::Window win = XCreateSimpleWindow(disp, root, x, y, w, h, depth, fg, bg);
  if (!win)
    throw WinException("Cannot create window", errno);

  long events = ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask;
  XSelectInput(disp, win, events);
  return win;
}

// Get available video mode at this monitor in std::vector<std::pair> format

VModes gdm::helpers::GetVideoModes(Display* disp, ::Window root)
{
  auto config = ptr::XRRGetScreenInfo(disp, root);

  if (!config)
    throw WinException("Can't recieve info about curr screen config", errno);
  
  int cnt_sizes;
  auto* sizes = XRRConfigSizes(config.get(), &cnt_sizes);
  
  VModes result {};
  for (auto i = 0; i < cnt_sizes; ++i)
    result.push_back(std::make_pair(sizes[i].width, sizes[i].height));
  return result;
}

// Returns number of current video mode in terms of RANDR extension

int gdm::helpers::GetCurrentVideoMode(Display* disp, ::Window root)
{
  Rotation rot;
  auto config = ptr::XRRGetScreenInfo(disp, root);  
  return XRRConfigCurrentConfiguration(config.get(), &rot);
}

// Changes video mode of the monitor

// We can change resolutions through Xrandr in three ways:
//  first  - with XRRSetScreenCongif
//  second - with XRRSetCrtcConfig
//  thirt  - with X86 extension
// Examples:
//  https://goo.gl/4zjY33
//  https://goo.gl/9yq81n
//  https://goo.gl/tuE48Z
// As "mode" used one of the values returned by GetVideoModes

int gdm::helpers::ChangeVideoMode(Display* disp, ::Window root, int mode)
{
  int dummy;
  if (!XQueryExtension(disp, "RANDR", &dummy, &dummy, &dummy))
    throw WinException("RandR extension didn't present", errno);

  auto config = ptr::XRRGetScreenInfo(disp, root);
  if (!(config.get()))
    throw WinException("Can't recieve info about curr screen config", errno);

  Rotation rot;
  int old_mode = XRRConfigCurrentConfiguration(config.get(), &rot);
  XRRSetScreenConfig(disp, config.get(), root, mode, rot, CurrentTime);
  return old_mode;
}

// Returns dimension of window itself with borders and elements

void gdm::helpers::GetWindowDimension(Display* disp, ::Window win, int* w, int* h)
{
  XWindowAttributes wa;
  XGetWindowAttributes(disp, win, &wa);
  *w = wa.width;
  *h = wa.height;
}

// Returns nearest video mode to requested

int gdm::helpers::FindNearestVideoMode(int w, int h)
{
  int mode = 0; // usually 0 is always present (current video mode)
  Display* disp = XOpenDisplay(NULL);
  
  int cnt = 0;
  int diff = std::numeric_limits<int>::max(); 
  auto modes = GetVideoModes(disp, XDefaultRootWindow(disp));
  auto each = [&diff, &mode, &cnt, w, h](auto& elem)
  { 
    auto curr = std::abs(w - elem.first) + std::abs(h - elem.second);
    if (curr < diff) {
      diff = curr;
      mode = cnt;
    }
    ++cnt;
  };
  std::for_each(modes.begin(), modes.end(), each);

  XCloseDisplay(disp);
  return mode;
}

// Returns number of video mode supported by randr extenstion
// Otherwise, returns -1

int gdm::helpers::FindVideoMode(int w, int h)
{
  Display* disp = XOpenDisplay(NULL);
  auto modes = GetVideoModes(disp, XDefaultRootWindow(disp));
  XCloseDisplay(disp);

  auto to_find = std::make_pair(w, h);
  auto finded = std::find(modes.begin(), modes.end(), to_find);

  if (finded != modes.end())
    return std::distance(modes.begin(), finded);
  else
    return -1;
}

void gdm::helpers::ChangeWindowName(Display* disp, ::Window win, const char* name)
{
  XStoreName(disp, win, name);
}

::Window gdm::helpers::GetFocusedWindow(Display* disp)
{
  int revert_to;
  ::Window wnd = 0;
  XGetInputFocus(disp, &wnd, &revert_to);
  return wnd;
}

// Makes cursor blank

void gdm::helpers::HideCursor(Display* disp, ::Window win)
{
  
  Pixmap blank;
  XColor dummy;
  char data[1] = {0};
  Cursor cursor;

  blank = XCreateBitmapFromData (disp, win, data, 1, 1);
  if (blank == None)
    throw WinException("Can't create cursor", errno);
  cursor = XCreatePixmapCursor(disp, blank, blank, &dummy, &dummy, 0, 0);
  XFreePixmap (disp, blank);

  XDefineCursor(disp, win, cursor);
}

void gdm::helpers::UnhideCursor(Display* disp, ::Window win)
{
  XUndefineCursor(disp, win);
}

Pos gdm::helpers::GetXYToMiddle(int w, int h)
{
  Display* disp = XOpenDisplay(NULL);
  int s = XDefaultScreen(disp);
  int dw = XDisplayWidth(disp, s);
  int dh = XDisplayHeight(disp, s);
  Pos p;
  p.x = (dw/2)-(w/2);
  p.y = (dh/2)-(h/2);
  XCloseDisplay(disp); 
  return p;
}

// Returns all Gl extensions (only after gl context creating)
// Format: strings through the space

const char* gdm::helpers::GetAllGlExtensions()
{
  return (const char*)glGetString(GL_EXTENSIONS);
}

// Returns all Glx extensions (any time)
// Format: strings through the space

const char* gdm::helpers::GetAllGlxExtensions(Display* disp)
{
  return glXQueryExtensionsString(disp, DefaultScreen(disp));
}

// Search if extensions is presents (in default extension list)

bool gdm::helpers::IsExtensionSupported(const char* extension)
{
  return IsExtensionSupported(GetAllGlExtensions(), extension);
}

// Search if extensions is presents (in custom extension list)
// (from https://goo.gl/M2v61N)

bool gdm::helpers::IsExtensionSupported(const char* extList, const char *extension)
{
  const char *start;
  const char *where, *terminator;
  
  // Extension names should not have spaces

  where = strchr(extension, ' ');
  if (where || *extension == '\0')
    return false;

  // It takes a bit of care to be fool-proof about parsing the
  // OpenGL extensions string. Don't be fooled by sub-strings, etc

  for (start=extList;;) {
    where = strstr(start, extension);

    if (!where)
      break;

    terminator = where + strlen(extension);

    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return true;

    start = terminator;
  }
  return false;
}

std::pair<int,int> gdm::helpers::GetGlContextVersion()
{
  GLint major {};
  GLint minor {};
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  return std::make_pair(major, minor);
}

// Returns in first - 1 is core profile, 2 is compatibility profile

std::pair<int,int> gdm::helpers::GetGlContextProfile()
{
  GLint profile {};
  GLint flags {};
  glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
  return std::make_pair(profile, flags);
}

std::pair<int,int> gdm::helpers::GetGlxVersion(Display* disp)
{
  GLint major {};
  GLint minor {};
  glXQueryVersion(disp, &major, &minor);
  return std::make_pair(major, minor);  
}

// Current context info
 
void gdm::helpers::PrintGlInfo(std::ostream& oss)
{
  std::string gl_ctx_profile = GetGlContextProfile().first == 1 ? "core" : "comp";
  oss << "GL Renderer: "  << glGetString(GL_RENDERER) << '\n'
      << "GL Version: "   << glGetString(GL_VERSION) << '\n'
      << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
}
bool gdm::helpers::SendToggleFullscreenNotify(Display* disp, ::Window root, ::Window win)
{
  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.display = disp;
  e.xclient.window = win;
  e.xclient.format = 32;
  e.xclient.message_type  = XInternAtom(disp, "_NET_WM_STATE", false);
  e.xclient.data.l[0]     = 2;  // 0 - off, 1 - on, 2 - toggle
  e.xclient.data.l[1]     = XInternAtom(disp, "_NET_WM_STATE_FULLSCREEN", false);
  e.xclient.data.l[3]     = 1;
  
  long mask = SubstructureNotifyMask | SubstructureRedirectMask;
  if (!XSendEvent(disp, root, false, mask, &e))
    return false;
  return true;
}

void gdm::helpers::SendCloseWindowNotify(Display* disp, ::Window root, ::Window win)
{
  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.display = disp;
  e.xclient.window = win;
  e.xclient.format = 32;
  e.xclient.message_type  = XInternAtom(disp, "WM_PROTOCOLS", true);
  e.xclient.data.l[0]     = XInternAtom(disp, "WM_DELETE_WINDOW", false);
  e.xclient.data.l[1]     = CurrentTime;
  
  long mask = NoEventMask;
  if (!XSendEvent(disp, win, false, mask, &e))
    throw WinException("Can't send wm_delete_window notify", errno);    
}

void gdm::helpers::SendToggleOnTopNotify(Display* disp, ::Window root, ::Window win)
{
  XEvent e;
  e.xclient.type = ClientMessage;
  e.xclient.serial = 0;
  e.xclient.send_event = True;
  e.xclient.display = disp;
  e.xclient.window  = win;
  e.xclient.format = 32;
  e.xclient.message_type = XInternAtom (disp, "_NET_WM_STATE", False);
  e.xclient.data.l[0] = 2;  // 0 - unset, 1 - set, 2 - toggle
  e.xclient.data.l[1] = XInternAtom (disp, "_NET_WM_STATE_ABOVE", false);
  e.xclient.data.l[2] = 0;
  e.xclient.data.l[3] = 0;
  e.xclient.data.l[4] = 0;

  long mask = SubstructureRedirectMask|SubstructureNotifyMask;
  if (!XSendEvent (disp, root, False, mask, &e))
    throw WinException("Can't send wm_toggle_on_top notify", errno);    
}

XRRScreenPtr gdm::helpers::ptr::XRRGetScreenInfo(Display* disp, ::Window root)
{
  XRRScreenConfiguration* p = ::XRRGetScreenInfo(disp, root);
  return XRRScreenPtr (p, xrr_deleter);
}

VisualPtr gdm::helpers::ptr::glXGetVisualFromFBConfig(Display* disp, GLXFBConfig& cfg)
{
  XVisualInfo* p = ::glXGetVisualFromFBConfig(disp, cfg);
  return VisualPtr (p, x_deleter<XVisualInfo>);
}

VisualPtr gdm::helpers::ptr::glXChooseVisual(Display* disp, int scr, int* data)
{
  XVisualInfo* p = ::glXChooseVisual(disp, scr, data);
  return VisualPtr (p, x_deleter<XVisualInfo>);
}

std::ostream& operator<<(std::ostream& oss, std::pair<int,int>& p)
{
  oss << p.first << '.' << p.second << '\n';
  return oss;
}
