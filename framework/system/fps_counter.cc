// *************************************************************
// File:    fps_counter.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "fps_counter.h"
#include "stdio.h"

#ifdef _WIN32
#include "windows.h"
#endif

// --public

void gdm::FpsCounter::Advance()
{
  timer_.End();
  ++curr_;
  time_passed_ += timer_.GetEndTime() - timer_.GetStartTime();
  if (time_passed_ >= 1000)
  {
    time_passed_ = 0;
    prev_ = curr_;
    curr_ = 0;
    data_ready_ = true;
  }
  timer_.Start(); 
}

long gdm::FpsCounter::ReadPrev()
{
  data_ready_ = false;
  return prev_;
}

bool gdm::FpsCounter::Ready() const
{
  return data_ready_;
}

// --helpers

void gdm::helpers::PrintFps(FpsCounter& fps)
{
  fps.Advance();
  if (fps.Ready())
  {
    char s[256];
    sprintf_s(s, 256, "%d\n", fps.ReadPrev());
    OutputDebugStringA(s);
  }    
}
