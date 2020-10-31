// *************************************************************
// File:    spin_lock.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "spin_lock.h"

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

gdm::SpinLock::SpinLock()
  : atomic_{}
{
  atomic_.clear();
}

void gdm::SpinLock::Lock()
{
  while(!TryLock())
  {
    _mm_pause();
  }
}

void gdm::SpinLock::Unlock()
{
  atomic_.clear(std::memory_order_release); 
}

bool gdm::SpinLock::TryLock()
{
  return !atomic_.test_and_set(std::memory_order_acquire);
}
