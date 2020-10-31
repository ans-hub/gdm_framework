// *************************************************************
// File:    vk_device_allocator.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_device_allocator.h"

#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_physical_device.h"
#include "system/assert_utils.h"

// --public

template<class T>
inline auto gdm::vk::DeviceAllocator::Allocate(Device* device, T buffer, VkMemoryPropertyFlagBits desired_flag) -> VkDeviceMemory
{
  VkMemoryRequirements memory_requirements = {};

  if constexpr (std::is_same_v<T, VkImage>)
    vkGetImageMemoryRequirements(*device, buffer, &memory_requirements);
  else
    vkGetBufferMemoryRequirements(*device, buffer, &memory_requirements);

  const auto& device_mem_props = device->GetPhysicalDevice().info_.device_mem_props_;
  uint memory_type_index = helpers::FindMemoryIndex(device_mem_props, memory_requirements.memoryTypeBits, desired_flag);  

  VkMemoryAllocateInfo allocate_info = {};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = memory_requirements.size;
  allocate_info.memoryTypeIndex = memory_type_index;

  VkDeviceMemory memory = {};
  VkResult res = vkAllocateMemory(*device, &allocate_info, HostAllocator::GetPtr(), &memory);
  ASSERTF(res == VK_SUCCESS, "vkAllocateMemory depth error %d\n", res);

  return memory;
}
