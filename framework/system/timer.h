// *************************************************************
// File:    timer.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TIMER_H
#define AH_GDM_TIMER_H

namespace gdm {

class Timer // ms internal dimension, external dt in sec
{
public:
  using slong = signed long;
  
  Timer() = default;
  explicit Timer(int frames_per_sec);

  void Wait();
  void Start();
  void End();
  slong GetStartTime() const;
  slong GetEndTime() const;
  float GetLastDt() const;
  slong GetCurrentClock() const;
  void SetMillisecondsToWait(int ms);

private:
  int ms_wait_;
  slong start_time_;
  slong end_time_;
  slong last_dt_;

}; // class Timer 

namespace timer_utils
{
  void WaitMs(signed long ms);

} // namespace timer_utils

}  // namespace gdm

#endif  // AH_GDM_TIMER_H

// Note : remainder - second = 1/1e3 milli, 1/e6 - micro, 1/1e9 nano
//        also there are problem with precision - needs to remake start/end
//        time to nanoseconds