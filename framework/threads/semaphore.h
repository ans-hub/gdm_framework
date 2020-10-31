// *************************************************************
// File:    semaphore.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_SEMAPHORE_H
#define AH_GDM_SEMAPHORE_H

#include <assert.h>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace gdm {

struct Semaphore
{
  explicit Semaphore(int value);

  void Post();
  void Wait();
  int Get() const;

private:
  std::atomic_int value_;
  std::condition_variable cv_;
  std::mutex mx_;

}; // struct Semaphore

}  // namespace gdm

#endif // AH_GDM_SEMAPHORE_H
