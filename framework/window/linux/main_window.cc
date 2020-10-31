// *************************************************************
// File:    main_window.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "main_window.h"

#include <iostream>
#include <memory>

gdm::MainWindow::MainWindow(
  const FBAttrs& fb_cfg, const CTXAttrs& ctx_cfg, int l, int t, int w, int h, const char* name)
  : XWindow()
  , glxver_ { helpers::GetGlxVersion(disp_) }
  , fbattr_ {fb_cfg}
  , ctxattr_{ctx_cfg}
  , glxself_{0}
  , fbcfgs_ {nullptr, 0}
  , clear_color_{0.f, 0.f, 0.f, 1.f}
  , clear_bits_{GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT}
{
  if (glxver_.first > 1 || (glxver_.first == 1 && glxver_.second >= 3))
  {
    fbcfgs_ = helpers::GetFBConfigs(disp_, fbattr_);
    int best = helpers::GetBestMSAAVisual(disp_, fbcfgs_);
    auto cfg = *(fbcfgs_[best]);
    auto fbvi = helpers::ChooseGlVisual(disp_, cfg);
    swa_ = helpers::GetDefaultWinAttribs(disp_, root_, fbvi.get());
    context_ = helpers::CreateGlContext(disp_, cfg, ctxattr_);
    self_ = helpers::CreateMainWindow(disp_, root_, fbvi.get(), swa_, l, t, w, h);

    if (helpers::FindFlagInFBAttrs(fb_cfg, GLX_DRAWABLE_TYPE, GLX_PBUFFER_BIT))
      pbuffer_ = helpers::CreatePBuffer(disp_, w, h, cfg);
    else
      pbuffer_ = 0;

    if (pbuffer_)
      glXMakeContextCurrent(disp_, self_, pbuffer_, context_);
    else {
      glxself_ = glXCreateWindow(disp_, cfg, self_, NULL);
      glXMakeContextCurrent(disp_, self_, glxself_, context_);
    }
  }
  else
  {
    auto fbvi = helpers::ChooseGlVisual(disp_);
    swa_ = helpers::GetDefaultWinAttribs(disp_, root_, fbvi.get());
    context_ = helpers::CreateGlContext(disp_, fbvi.get());
    self_ = helpers::CreateMainWindow(disp_, root_, fbvi.get(), swa_, l, t, w, h);
    glXMakeCurrent(disp_, self_, context_);
  }

  PrintGlInfo(std::cerr);  
  ChangeWindowName(disp_, self_, name);
  this->Show();
  this->Move(l, t);
  this->NotifyWhenClose();

  helpers::GetWindowDimension(disp_, self_, &width_, &height_);
}

gdm::MainWindow::MainWindow(MainWindow&& rhs)
  : glxver_ { std::move(rhs.glxver_) }
  , fbattr_ { std::move(rhs.fbattr_) }
  , ctxattr_{ std::move(rhs.ctxattr_) }
  , context_{ rhs.context_ }
  , pbuffer_{ rhs.pbuffer_ }
  , glxself_{ rhs.glxself_ }
  , fbcfgs_ { std::move(rhs.fbcfgs_) }
  , wa_ { std::move(rhs.wa_) }
  , swa_ { std::move(rhs.swa_) }
{
  context_ = nullptr;
  pbuffer_ = 0;
  glxself_ = 0;
}

gdm::MainWindow::~MainWindow()
{
  if (fbcfgs_.front())
    XFree(fbcfgs_.front());
  if (glxself_)
    glXDestroyWindow(disp_, glxself_);  
  if (pbuffer_)
    glXDestroyPbuffer(disp_, pbuffer_);
  glXMakeCurrent(disp_, None, NULL);
  glXDestroyContext(disp_, context_);
}

void gdm::MainWindow::VerticalSync(bool on)
{
  helpers::ToggleVerticalSync(disp_, self_, on);
}

void gdm::MainWindow::Clear()
{
  glClearColor(clear_color_[0], clear_color_[1], clear_color_[2], 1.f);
  glClearDepth(1.f);
  glClear(clear_bits_);
}

void gdm::MainWindow::Redraw()
{ 
  glXSwapBuffers(disp_, self_);
}

void gdm::MainWindow::Exposed()
{
  XGetWindowAttributes(disp_, self_, &wa_);
  glViewport(0, 0, wa_.width, wa_.height);
}

MainWindow gl_window::MakeCentered(int w, int h, const char* caption)
{
  Pos pos = helpers::GetXYToMiddle(w, h);
  return MainWindow(helpers::GetDefaultFBAttribs42(),
                    helpers::GetGlContextAttributes42(),
                    pos.x, pos.y, w, h, caption);
}

MainWindow gl_window::MakeFullscreen(int w, int h, const char* caption)
{
  Pos pos = helpers::GetXYToMiddle(w, h);
  int mode = helpers::FindVideoMode(w, h);
  MainWindow win (helpers::GetDefaultFBAttribs42(),
                  helpers::GetGlContextAttributes42(),
                  pos.x, pos.y, w, h, caption);
  win.ToggleFullscreen(mode);
  return win;
}

// Note 1: only after context_ creating we may use opengl functions
// Note 2: when we draw in msaa something wrong with colors... I suppose
//         this is depends on compatibility profile of gl
