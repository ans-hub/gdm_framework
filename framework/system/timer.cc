// *************************************************************
// File:    timer.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "timer.h"

#include <chrono>

#if defined _WIN32
#include <windows.h>
#endif

// --private

#if defined _WIN32
static int nanosleep(timespec* ts0, timespec* ts1)
{
  HANDLE timer;
  LARGE_INTEGER li;
  if(!(timer = ::CreateWaitableTimer(nullptr, true, nullptr)))
	  return -1;
  li.QuadPart = -(ts0->tv_sec + (ts1->tv_nsec / 100));  // 100 ns units
  if(!::SetWaitableTimer(timer, &li, 0, nullptr, nullptr, false))
  {
		::CloseHandle(timer);
	  return -1;
	}
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
  return 0;
}
#endif

// --public

gdm::Timer::Timer(int frames_per_sec)
  : wait_mu_{1'000'000 / frames_per_sec}
  , start_time_mu_{}
  , end_time_mu_{}
  , last_dt_mu_{}
{ }

void gdm::Timer::Start()
{
  start_time_mu_ = GetCurrentClockMu();
}

void gdm::Timer::End()
{
  end_time_mu_ = GetCurrentClockMu();
  last_dt_mu_ = end_time_mu_ - start_time_mu_;
}

auto gdm::Timer::GetStartTimeMu() const -> gdm::Timer::slong
{
  return start_time_mu_;
}

auto gdm::Timer::GetEndTimeMu() const -> gdm::Timer::slong
{
  return end_time_mu_;
}

float gdm::Timer::GetLastDt() const
{
  if (last_dt_mu_ < 1'000'000)
    return static_cast<float>(last_dt_mu_) / 1'000'000.f;
  else
    return static_cast<float>(wait_mu_) / 1'000'000.f;
}

auto gdm::Timer::GetCurrentClockMu() const -> gdm::Timer::slong
{
  return static_cast<slong>(
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()).count()
  );
}

// --private

void gdm::Timer::Wait()
{
  const slong last_dt_mu = end_time_mu_ - start_time_mu_;
  slong wait_for_mu = wait_mu_ - last_dt_mu;
  if (wait_for_mu < 0)
    wait_for_mu = 0;

  timespec ts;
  ts.tv_sec  = wait_for_mu / 1'000'000;
  ts.tv_nsec = static_cast<long>(wait_for_mu) * 1'000;
  while(nanosleep(&ts, &ts) == -1) { }
  [[maybe_unused]] auto have_wait = GetCurrentClockMu() - end_time_mu_;
}
