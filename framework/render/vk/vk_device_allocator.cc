// *************************************************************
// File:    vk_device_allocator.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_device_allocator.h"

#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_physical_device.h"
#include "system/assert_utils.h"

// --public

void gdm::vk::DeviceAllocator::Free(Device* device, VkDeviceMemory memory)
{
  vkFreeMemory(*device, memory, HostAllocator::GetPtr());
}
