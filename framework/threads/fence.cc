// *************************************************************
// File:    fence.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "fence.h"

//--public

gdm::Fence::Fence()
  : value_{0}
  , cv_{}
  , mx_{}
{ }

void gdm::Fence::WaitOnValue(int value)
{
  std::unique_lock<std::mutex> lock(mx_);
  if (value_ < value)
    cv_.wait(lock, [this, value](){ return value_ >= value; });
}
