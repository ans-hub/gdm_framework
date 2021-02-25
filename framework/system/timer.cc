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
  : ms_wait_{1000/frames_per_sec}
  , start_time_{}
  , end_time_{}
  , last_dt_{}
{ }

void gdm::Timer::Start()
{
  start_time_ = GetCurrentClock();
}

void gdm::Timer::End()
{
  end_time_ = GetCurrentClock();
  last_dt_ = end_time_ - start_time_;
}

auto gdm::Timer::GetStartTime() const -> gdm::Timer::slong
{
  return start_time_;
}

auto gdm::Timer::GetEndTime() const -> gdm::Timer::slong
{
  return end_time_;
}

float gdm::Timer::GetLastDt() const
{
  if (last_dt_ < 1000)
    return last_dt_ / 1000.f;
  else
    return static_cast<float>(ms_wait_);
}

auto gdm::Timer::GetCurrentClock() const -> gdm::Timer::slong
{
  return static_cast<slong>(
    std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()).count()
  );
}

void gdm::Timer::SetMillisecondsToWait(int ms)
{
  ms_wait_ = ms;
}

// --private

void gdm::Timer::Wait()
{
  slong last_dt = end_time_ - start_time_;
  slong wait_for = ms_wait_ - last_dt > 0 ? ms_wait_ - last_dt : 0;
  timespec ts;
  ts.tv_sec  = wait_for / 1000;
  ts.tv_nsec = wait_for * 1000000;
  while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) { }
  end_time_ = GetCurrentClock();
  last_dt_ = end_time_ - start_time_;
}


// --helpers

void gdm::timer_utils::WaitMs(signed long ms)
{
  timespec ts;
  ts.tv_sec  = ms / 1000;
  ts.tv_nsec = ms * 1000000;
  while ((nanosleep(&ts, &ts) == -1) && (errno == EINTR)) { }
}
