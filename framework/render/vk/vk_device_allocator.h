// *************************************************************
// File:    vk_device_allocator.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_ALLOCATOR_DEVICE_H
#define GM_VK_ALLOCATOR_DEVICE_H

#include "render/vk/vk_defines.h"

namespace gdm::vk {

struct Device;

using Allocation = VkDeviceMemory;

struct DeviceAllocator
{
  template<class T>
  static auto Allocate(Device* device, T buffer, VkMemoryPropertyFlagBits desired_flag) -> Allocation;
  static void Free(Device* device, Allocation memory);

}; // struct DeviceAllocator

} // namespace gdm::vk

#include "vk_device_allocator.inl"

#endif // GM_VK_ALLOCATOR_DEVICE_H
