// *************************************************************
// File:    timer.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TIMER_H
#define AH_GDM_TIMER_H

namespace gdm {

class Timer
{
public:
  using slong = signed long long;
  
  Timer() = default;
  explicit Timer(int frames_per_sec);

  void Wait();
  void Start();
  void End();
  auto GetStartTimeMu() const -> slong;
  auto GetEndTimeMu() const -> slong;
  auto GetLastDt() const -> float;
  auto GetCurrentClockMu() const -> slong;

private:
  slong wait_mu_;
  slong start_time_mu_;
  slong end_time_mu_;
  slong last_dt_mu_;

}; // class Timer 

}  // namespace gdm

#endif  // AH_GDM_TIMER_H

// Note : remainder - second = 1/1e3 milli, 1/e6 - micro, 1/1e9 nano
//        also there are problem with precision - needs to remake start/end
//        time to nanoseconds