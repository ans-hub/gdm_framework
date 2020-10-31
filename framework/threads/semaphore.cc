// *************************************************************
// File:    semaphore.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "semaphore.h"

gdm::Semaphore::Semaphore(int value)
  : value_{value}
  , cv_{}
  , mx_{}
{
  assert(value >= 0);
}

void gdm::Semaphore::Post()
{
  {
    std::unique_lock<std::mutex> lock(mx_);
    assert(value_ >= 0);
    ++value_;
  }
  cv_.notify_all();
}

void gdm::Semaphore::Wait()
{
  std::unique_lock<std::mutex> lock(mx_);
  assert(value_ >= 0);
  if (value_ == 0)
    cv_.wait(lock, [this](){ return value_ > 0; });
  --value_;
}

int gdm::Semaphore::Get() const
{
  return value_;
}
