// *************************************************************
// File:    fence.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_FENCE_H
#define AH_GDM_FENCE_H

#include <assert.h>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace gdm {

struct Fence
{
  Fence();

  auto GetValue() const -> int { return value_; };
  void WaitOnValue(int value);

private:
  std::atomic_int value_;
  std::condition_variable cv_;
  std::mutex mx_;

}; // struct Fence

}  // namespace gdm

#endif // AH_GDM_FENCE_H
