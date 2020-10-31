// *************************************************************
// File:    vk_fence.h cpu-gpu
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_FENCE_H
#define GM_VK_FENCE_H

#include <limits>

#include "render/defines.h"

namespace gdm::vk {

struct Fence
{
  Fence(VkDevice device, gfx::FenceFlags flags = 0);
  ~Fence();
  Fence(const Fence& other) = delete;
  const Fence& operator=(const Fence& other) = delete;
  operator VkFence() const { return fence_; }

  void WaitSignalFromGpu(uint64_t timeout = UINT64_MAX);
  void Reset();

public:
  constexpr static VkFence null = VK_NULL_HANDLE;

private:
  auto CreateFence(gfx::FenceFlags flags) -> VkFence;

private:
  VkDevice device_;
  VkFence fence_;

}; // struct Fence

} // namespace gdm::vk

#endif // GM_VK_FENCE_H
