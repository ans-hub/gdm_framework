// *************************************************************
// File:    main_window.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "window/enums.h"
#include "window/aliases.h"
#include "window/helpers.h"

#ifndef AH_GDM_LINUX_WINDOW_H
#define AH_GDM_LINUX_WINDOW_H

namespace gdm {

struct MainWindow : public XWindow
{
  MainWindow(const FBAttrs&, const CTXAttrs&, int l, int t, int w, int h, const char* name);
  MainWindow(const MainWindow&) = delete;
  MainWindow(MainWindow&&);
  ~MainWindow() override;

  virtual void Clear() override;
  virtual void Redraw() override;
  virtual void Exposed() override;

  void SetClearColor(float cols[4]) { memcpy(clear_color_,cols, 4); }
  void SetClearBit(std::size_t clear_bit) { clear_bits_ &= clear_bit; }
  void UnsetClearBit(std::size_t clear_bit) { clear_bits_ ^= clear_bit; }
  void VerticalSync(bool switch_on);

private:
  std::pair<int,int> glxver_;
  FBAttrs fbattr_;
  CTXAttrs ctxattr_;
  GLXContext context_;
  GLXPbuffer pbuffer_;
  GLXWindow glxself_;
  FBConfigs fbcfgs_;
  float clear_color_[4];
  std::size_t clear_bits_;

  WAttribs wa_;
  SWAttribs swa_;

}; // class MainWindow

namespace gl_window
{
  GlWindow MakeCentered(int w, int h, const char* caption);
  GlWindow MakeFullscreen(int w, int h, const char* caption);

} // namespace gl_window

} // namespace gdm

#endif  // AH_GDM_LINUX_WINDOW_H
