// *************************************************************
// File:    vk_host_allocator.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_ALLOCATOR_HOST_H
#define GM_VK_ALLOCATOR_HOST_H

#include "render/vk/vk_defines.h"

namespace gdm::vk {

struct HostAllocator
{
  static auto Allocate(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope) -> void*;
  static auto Reallocate(void* user_data, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope) -> void*;
  static void Free(void* user_data, void* ptr);

public:
  static auto GetPtr() -> VkAllocationCallbacks* { return &allocator_; }

private:
  static auto Initialize() -> VkAllocationCallbacks;
  static VkAllocationCallbacks allocator_;
};

} // namespace gdm::vk

#endif // GM_VK_ALLOCATOR_HOST_H
