// *************************************************************
// File:    main_window.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_WIN_WINDOW_H
#define AH_GDM_WIN_WINDOW_H

#include <Windows.h>

#include "window/win/dx_input.h"

namespace gdm {

struct MainWindow
{
  enum EWindowProps
  {
    FULLSCREEN  = 1 << 1,
    CENTERED    = 1 << 2

  }; // enum EWindowProps

  MainWindow(int w, int h, const char* name, unsigned int win_props = 0);

  void ToggleFullscreen();
  void ProcessInput(DxInput& input);
  void SetResized(int w, int h) { width_ = w; height_ = h; resized_ = true; }
  bool IsResized() const { bool r = resized_; resized_ = false; return r; }
  
  auto GetHandle() const { return hndl_; }
  auto GetWidth() const -> int { return width_; }
  auto GetHeight() const -> int { return height_; }
  auto GetAspectRatio() const -> float { return (float)width_ / (float)height_; }

private:  
  HWND InitializeWindow();
  void CenterWindow(); 

private:
  const unsigned int k_dx_icon_ = 102;  // question icon
  const char* k_class_name_ = "MainWindow class";
  LONG hwnd_style = 0;
  LONG hwnd_style_ex = 0;

private:
  int width_;
  int height_;
  const char* name_;
  HINSTANCE h_inst_;
  HWND hndl_;
  int cmd_showidth_;
  bool full_screen_;
  mutable bool resized_;

}; // struct MainWindow

} // namespace gdm

#endif // AH_GDM_WIN_WINDOW_H