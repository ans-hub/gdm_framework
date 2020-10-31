// *************************************************************
// File:    scoperd_timer.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_SCOPED_TIMER_H
#define AH_GDM_SCOPED_TIMER_H

#include <chrono>

namespace gdm {

struct ScopedTimer
{
  using value_type = std::milli;

  explicit ScopedTimer(double& return_ref);
  ~ScopedTimer();

private:
  double& return_ref_;
  std::chrono::high_resolution_clock::time_point start_value_;

}; // class ScopedTimer

inline explicit ScopedTimer::ScopedTimer(double& return_ref)
  : return_ref_{return_ref}
  , start_value_{std::chrono::high_resolution_clock::now()}
{ }

inline ScopedTimer::~ScopedTimer()
{
  return_ref_ += (std::chrono::high_resolution_clock::now() - start_value_).count();
}

} // namespace gdm

#endif // AH_GDM_SCOPED_TIMER_H