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
  FpsCounter() = default;
  
  void Advance();
  long ReadPrev();
  bool Ready() const;

private:
  Timer timer_;
  int   curr_;
  int   prev_;
  bool  data_ready_;
  long  time_passed_;
  
}; // struct FpsCounter

}  // namespace gdm

namespace gdm::helpers {

  void PrintFps(FpsCounter& fps);

}  // namespace gdm::helpers

#endif  // AH_GDM_FPS_COUNTER_H