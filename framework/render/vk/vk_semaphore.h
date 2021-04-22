// *************************************************************
// File:    vk_semaphore.h gpu-gpu
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_SEMAPHORE_H
#define GM_VK_SEMAPHORE_H

#include <limits>
#include <vector>

#include "render/defines.h"

namespace gdm::vk {

// api::Semaphore::null
// api::Semaphores::empty
// api::Semaphores{sem1, sem2}
// api::Fence::null

struct Semaphore
{ 
  Semaphore(VkDevice device, gfx::SemaphoreFlags flags = 0);
  ~Semaphore();

  Semaphore(const Semaphore& sem) = delete;
  const Semaphore& operator=(const Semaphore& sem) = delete;
  operator VkSemaphore() const { return semaphore_; }

public:
  constexpr static VkSemaphore null = VK_NULL_HANDLE;

private:
  auto CreateSemaphore(gfx::SemaphoreFlags flags) -> VkSemaphore;
  void Signal();
  void Reset();

private:
  VkDevice device_;
  VkSemaphore semaphore_;

}; // struct Semaphore

struct Semaphores
{
  Semaphores(std::initializer_list<VkSemaphore> list)
  {
    for (VkSemaphore sem : list)
      semaphores_.push_back(sem);
  }
  operator std::vector<VkSemaphore>() const { return semaphores_; }
  inline static const std::vector<VkSemaphore> empty = {};
  std::vector<VkSemaphore> semaphores_;

}; // struct Semaphores

} // namespace gdm::vk

#endif // GM_VK_SEMAPHORE_H
