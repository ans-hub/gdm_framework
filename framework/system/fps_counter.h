// *************************************************************
// File:    fps_counter.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_FPS_COUNTER_H
#define AH_GDM_FPS_COUNTER_H

#include "timer.h"

namespace gdm {

struct FpsCounter
{
  using slong = signed long long;

  FpsCounter() = default;
  
  void Advance();
  auto ReadPrev() const -> slong;
  bool Ready() const;

private:
  Timer timer_;
  int curr_;
  int prev_;
  mutable bool data_ready_;
  slong time_passed_;
  
}; // struct FpsCounter

}  // namespace gdm

namespace gdm::helpers {

  void PrintFps(FpsCounter& fps);

}  // namespace gdm::helpers

#endif  // AH_GDM_FPS_COUNTER_H