// *************************************************************
// File:    spin_lock.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_SPIN_LOCK_H
#define AH_GDM_SPIN_LOCK_H

#include <atomic>

namespace gdm {

struct SpinLock
{
  SpinLock();

  void Lock();
  bool TryLock();
  void Unlock();

private:
  std::atomic_flag atomic_;

}; // struct SpinLock

}  // namespace gdm

#endif // AH_GDM_SPIN_LOCK_H
