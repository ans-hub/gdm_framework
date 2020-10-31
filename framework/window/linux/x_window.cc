// *************************************************************
// File:    x_window.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "x_window.h"

// --public

gdm::XWindow::XWindow()
  : disp_{ XOpenDisplay(NULL) }
  , root_{}
  , self_{}
  , event_{}
  , fullscreen_{false}
  , vmode_{-1}
  , width_{0}
  , height_{0}
{
  if (!disp_)
    throw WinException("XOpenDisplay failed", errno);

  root_ = XDefaultRootWindow(disp_);

  auto ask_wm_notify_closing = [&]()
  {
    wm_protocols_     = XInternAtom(disp_, "WM_PROTOCOLS", false);
    wm_delete_window_ = XInternAtom(disp_, "WM_DELETE_WINDOW", false);
  }();
}

gdm::XWindow::XWindow(XWindow&& rhs)
  : disp_{ rhs.disp_ }
  , root_{ rhs.root_ }
  , self_{ rhs.self_ }
  , event_{ rhs.event_ }
  , fullscreen_{ rhs.fullscreen_ }
  , vmode_{ rhs.vmode_ }
  , width_{ rhs.width_ }
  , height_{ rhs.height_ }
  , wm_protocols_ { rhs.wm_protocols_ }
  , wm_delete_window_ { rhs.wm_delete_window_ }
{
  rhs.disp_ = nullptr;
  rhs.root_ = 0;
  rhs.self_ = 0;
  rhs.fullscreen_ = false;
  rhs.vmode_ = 0;
  rhs.width_ = 0;
  rhs.height_ = 0;
  rhs.wm_protocols_ = 0;
  rhs.wm_delete_window_ = 0;
}

gdm::XWindow::~XWindow()
{
  if (vmode_ != -1)
    helpers::ChangeVideoMode(disp_, root_, vmode_);
  if (self_) {
    XDestroyWindow(disp_, self_);
    XFlush(disp_);
  }
  if (disp_)
    XCloseDisplay(disp_);
}

void gdm::XWindow::Show()
{
  XMapWindow(disp_, self_);
  XFlush(disp_);
}

void gdm::XWindow::Hide()
{
  XUnmapWindow(disp_, self_);
  XFlush(disp_);
}

void gdm::XWindow::Move(int x, int y)
{
  XMoveWindow(disp_, self_, x, y);
}

void gdm::XWindow::Close()
{
  helpers::SendCloseWindowNotify(disp_, root_, self_);
}

void gdm::XWindow::HideCursor()
{
  helpers::HideCursor(disp_, self_);
}

void gdm::XWindow::UnhideCursor()
{
  helpers::UnhideCursor(disp_, self_);
}

void gdm::XWindow::SetFocus()
{
  XRaiseWindow(disp_, self_);
  XSetInputFocus(disp_, self_, RevertToNone, CurrentTime);
}

bool gdm::XWindow::ToggleFullscreen()
{
  int curr = helpers::GetCurrentVideoMode(disp_, root_);
  return this->ToggleFullscreen(curr);  
}

bool gdm::XWindow::ToggleFullscreen(int mode)
{
  if (mode < 0)
    return false;

  this->Move(0,0);
  vmode_ = helpers::ChangeVideoMode(disp_, root_, mode);
  helpers::GetWindowDimension(disp_, root_, &width_, &height_);

  int wait = 1000;  // todo: make more smart way to wait while resolution will be changed
  do {
    timespec ts;        
    ts.tv_sec  = wait / 1000;
    ts.tv_nsec = wait * 1000000;
    while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) { }

  } while (GrabSuccess != XGrabPointer(
        disp_, self_, True, None, GrabModeAsync, GrabModeAsync,
        self_, None, CurrentTime));
  SetFocus();
  XWarpPointer(disp_, None, root_, 0, 0, 0, 0, 0, 0); // todo: place in the middle of the scr

  bool result = helpers::SendToggleFullscreenNotify(disp_, root_, self_);
  fullscreen_ ^= result;

  if (!fullscreen_)
    XUngrabPointer(disp_, CurrentTime);
  
  return true;
}

void gdm::XWindow::ToggleOnTop()
{
  helpers::SendToggleOnTopNotify(disp_, root_, self_);
}

bool gdm::XWindow::IsClosed()
{
  if (XCheckTypedWindowEvent(disp_, self_, ClientMessage, &event_)) {
    if (event_.xclient.message_type == wm_protocols_ &&
        event_.xclient.data.l[0] == (int)wm_delete_window_) {
      return true;
    }        
  }
  return false;
}
//  Exposed() - only if window was moved, overlapperd, etc
//  Up window (need to hide wm ontop elements after change resolution)
//  Redraw()  - every time

void gdm::XWindow::Render()
{
  if (XCheckWindowEvent(disp_, self_, ExposureMask, &event_)) {
    Exposed();
    helpers::GetWindowDimension(disp_, self_, &width_, &height_);
  }
  if (fullscreen_ && XCheckTypedWindowEvent(disp_, self_, VisibilityNotify, &event_)) {
    XRaiseWindow(disp_, self_);
    helpers::GetWindowDimension(disp_, self_, &width_, &height_);
  }
  Redraw();
}

auto gdm::XWindow::GetNextEvent()
{
  XNextEvent(disp_, &event_);
  switch (event_.type) {
    case Expose         : return WinEvent::EXPOSE;
    case KeyPress       : return WinEvent::KEYPRESS;
    case KeyRelease     : return WinEvent::KEYRELEASE;
    case ButtonPress    : return WinEvent::MOUSEPRESS;
    case ButtonRelease  : return WinEvent::MOUSERELEASE;
    case MotionNotify   : return WinEvent::MOUSEMOVE;
    default             : return WinEvent::NONSENCE;
  }
}

Btn gdm::XWindow::ReadKeyboardBtn(BtnType t) const
{
  XEvent event;
  auto buf = Btn::NONE;
  long type = 1L << static_cast<int>(t);
  if (XCheckWindowEvent(disp_, self_, type, &event))
  {
    auto key = XkbKeycodeToKeysym(disp_, event.xkey.keycode, 0, 0);
    char buff[32];
    XLookupString(&event.xkey, buff, 32, &key, NULL);  // see note below
    buf = static_cast<Btn>(key);
  }
  return buf;

  // Note : This string is necessary if somewho press key in layout
  // differ than ISO Latin-1
}

Btn gdm::XWindow::ReadMouseBtn(BtnType t) const
{
  XEvent event;
  auto buf = Btn::NONE;
  long type = 1L << static_cast<int>(t);
  if (XCheckWindowEvent(disp_, self_, type, &event))
  {
    buf = static_cast<Btn>(event.xbutton.button + kMouseBtnOffset);
  }
  return buf;
}

// Returns mouse pos every time its requered. When required non-glob
// there are may be situation when first result returns position without
// window decoration, and other results are with it. Thus more preffered
// use glob == true

Pos gdm::XWindow::ReadMousePos(bool glob) const
{
  Window root_ret;
  Window child_ret;
  int x_rel {0};
  int y_rel {0};
  int x_win {0};
  int y_win {0};
  unsigned int mask;

  XQueryPointer(
    disp_, self_, &root_ret, &child_ret, &x_rel, &y_rel, &x_win, &y_win, &mask);
  if (glob)
    return Pos{x_rel, y_rel};
  else
    return Pos{x_win, y_win};
}

// Returns correct mouse pos only if moved, else default value for Pos

Pos gdm::XWindow::ListenMousePos() const
{
  // Other versions - http://www.rahul.net/kenton/perf.html

  Pos buf {};
  if (XCheckWindowEvent(disp_, self_, PointerMotionMask, &event_))
  {
    XSync(disp_, true);
    buf.x = event_.xmotion.x;
    buf.y = event_.xmotion.y;
  }
  return buf;
}

// Sets mouse pos relative to root
// todo: incorrect working

void gdm::XWindow::MoveMousePos(int x, int y)
{
  SetFocus();
  XWarpPointer(disp_, self_, self_, 0, 0, 0, 0, x, y);
}

// From here: https://goo.gl/W3zqgh

bool gdm::XWindow::IsKeyboardBtnPressed(KbdBtn btn) const
{
  if (btn == KbdBtn::NONE)
    return false;

  KeyCode keycode = XKeysymToKeycode(disp_, static_cast<KeySym>(btn));
  
  if (keycode) {
    char keys[32];
    XQueryKeymap(disp_, keys);
    return (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
  }
  else
    return false;
}

// From here: https://goo.gl/W3zqgh

bool gdm::XWindow::IsMouseBtnPressed(MouseBtn btn) const
{
  Window root_ret;
  Window child_ret;
  int x_rel {0};
  int y_rel {0};
  int x_win {0};
  int y_win {0};
  unsigned int btns;

  XQueryPointer(
    disp_, self_, &root_ret, &child_ret, &x_rel, &y_rel, &x_win, &y_win, &btns);

  switch (btn)
  {
    case MouseBtn::LMB:     return btns & Button1Mask;
    case MouseBtn::RMB:     return btns & Button3Mask;
    case MouseBtn::MMB:     return btns & Button2Mask;
    case MouseBtn::WH_UP:   return false;
    case MouseBtn::WH_DWN:  return false;
    default:                return false;
  }
  return false;
}

// Ask WM to notify when it should close the window

void gdm::XWindow::NotifyWhenClose()
{
  XSetWMProtocols(disp_, self_, &wm_delete_window_, 1);
}

// Note : XCheckWindowEvent doesn't wait for next event (like XNextEvent)
// but check if event is present. Function used that function only works
// with masked events. For not masked events - XCheckTypedWindowEvent()