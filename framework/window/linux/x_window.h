// *************************************************************
// File:    x_window.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "unistd.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <X11/extensions/Xrandr.h>

#include "window/enums.h"
#include "window/aliases.h"
#include "window/exceptions.h"
#include "window/helpers.h"

#ifndef GDM_X_WINDOW_H
#define GDM_X_WINDOW_H

namespace gdm {

struct XWindow
{
  XWindow();
  XWindow(const XWindow&) = delete;
  XWindow& operator=(const XWindow&) = delete;
  XWindow(XWindow&&);
  virtual ~XWindow();

  void Show();
  void Hide();
  void Move(int x, int y);
  void Close();
  void HideCursor();
  void UnhideCursor();
  void SetFocus();
  bool ToggleFullscreen();
  bool ToggleFullscreen(int mode);
  void ToggleOnTop();
  bool IsClosed();
  
  // Render

  void Render();
  virtual void Clear() = 0;
  virtual void Redraw()  = 0;
  virtual void Exposed() = 0;

  // Input handlers

  auto GetNextEvent();
  auto ReadKeyboardBtn(BtnType) const -> Btn;
  auto ReadMouseBtn(BtnType) const -> Btn;
  auto ReadMousePos(bool glob = true) const -> Pos;
  auto ListenMousePos() const -> Pos;
  void MoveMousePos(int x, int y);
  bool IsKeyboardBtnPressed(KbdBtn) const;
  bool IsMouseBtnPressed(MouseBtn) const;
  
  // Helpers

  auto Width() const -> int { return width_; }
  auto Height() const -> int { return height_; }
  auto Disp() { return disp_; }
  auto Self() { return self_; }

protected:
  Display* disp_;
  ::Window root_;
  ::Window self_;
  mutable XEvent event_;
  bool fullscreen_;
  int vmode_;               // see Xrandr vmodes
  int width_;
  int height_;

protected:
  void NotifyWhenClose();   // see note #1
  
private:
  Atom wm_protocols_;       // catch closes
  Atom wm_delete_window_;   // button event of window_manager

}; // class XWindow

}  // namespace gdm

#endif  // GDM_X_WINDOW_H

// Note #1 : every child should call this in constructor if it
// wants to recieve notifies about closing window. It is necessary
// do in child class since WM notify about closing concrete window.
// We can`t call its in the BaseWindow, since self_ is not created
// yet