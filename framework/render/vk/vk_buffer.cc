// *************************************************************
// File:    vk_buffer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_buffer.h"

#include "render/vk/vk_device.h"
#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device_allocator.h"

#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public

gdm::vk::Buffer::Buffer(Device* device, uint size, gfx::BufferUsage usage, gfx::MemoryType memory_type)
  : device_{device}
  , buffer_usage_{static_cast<VkBufferUsageFlagBits>(usage)}
  , memory_type_{static_cast<VkMemoryPropertyFlagBits>(memory_type)}
  , buffer_info_{}
  , buffer_{VK_NULL_HANDLE}
  , buffer_memory_{VK_NULL_HANDLE}
  , mapped_region_{nullptr}
{
  // todo: if uniform ,then assert on alignong
  buffer_info_.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info_.size = size;
  buffer_info_.usage = buffer_usage_;
  buffer_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffer_info_.queueFamilyIndexCount = 0;
  buffer_info_.pQueueFamilyIndices = NULL;
  
  VkResult res = vkCreateBuffer(*device_, &buffer_info_, HostAllocator::GetPtr(), &buffer_);
  ASSERTF(res == VK_SUCCESS, "vkCreateBuffer error %d\n", res);
 
  buffer_memory_ = DeviceAllocator::Allocate(device_, buffer_, memory_type_);
  res = vkBindBufferMemory(*device_, buffer_, buffer_memory_, 0);
  ASSERTF(res == VK_SUCCESS, "vkBindBufferMemory failed %d", res);
}

gdm::vk::Buffer::~Buffer()
{
  if (mapped_region_)
    Unmap();
  vkDestroyBuffer(*device_, buffer_, HostAllocator::GetPtr());
  DeviceAllocator::Free(device_, buffer_memory_);
}

void gdm::vk::Buffer::Map()
{
  ASSERTF(mapped_region_ == nullptr, "Buffer already mapped");
  ASSERTF(bits::HasFlag(memory_type_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), "Copy is not allowed");

  VkResult res = vkMapMemory(*device_, buffer_memory_, 0, buffer_info_.size, 0, &mapped_region_);
  ASSERTF(res == VK_SUCCESS, "vkMapMemory failed %d", res);
}
 
void gdm::vk::Buffer::Map(uint offset, uint size)
{
  ASSERTF(mapped_region_ == nullptr, "Buffer already mapped");
  ASSERTF(bits::HasFlag(memory_type_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), "Copy is not allowed");

  VkResult res = vkMapMemory(*device_, buffer_memory_,static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size), 0, &mapped_region_);
  ASSERTF(res == VK_SUCCESS, "vkMapMemory failed %d", res);
}

void gdm::vk::Buffer::Unmap()
{
  ASSERTF(mapped_region_ != nullptr, "Buffer already mapped");
  vkUnmapMemory(*device_, buffer_memory_);
  mapped_region_ = nullptr;
}
