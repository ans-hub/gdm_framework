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

// --public Resource<Buffer>

gdm::gfx::Resource<gdm::api::Buffer>::Resource(api::Device* device, uint size)
  : res_ { GMNew api::Buffer() }
{
  // todo: if uniform ,then assert on aligning

  res_->device_ = device;
 
  res_->flush_range_alignment_ = device->GetPhysicalDevice().info_.device_props_.limits.nonCoherentAtomSize;
 
  res_->buffer_info_.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  res_->buffer_info_.size = size;
  res_->buffer_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  res_->buffer_info_.queueFamilyIndexCount = 0;
  res_->buffer_info_.pQueueFamilyIndices = NULL;
}

gdm::gfx::Resource<gdm::api::Buffer>::~Resource()
{
  VkResult res = vkCreateBuffer(*res_->device_, &res_->buffer_info_, api::HostAllocator::GetPtr(), &res_->buffer_);
  ASSERTF(res == VK_SUCCESS, "vkCreateBuffer error %d\n", res);
 
  res_->buffer_memory_ = api::DeviceAllocator::Allocate(res_->device_, res_->buffer_, res_->memory_type_);
  res = vkBindBufferMemory(*res_->device_, res_->buffer_, res_->buffer_memory_, 0);
  ASSERTF(res == VK_SUCCESS, "vkBindBufferMemory failed %d", res);
}

auto gdm::gfx::Resource<gdm::api::Buffer>::AddUsage(gfx::BufferUsage usage) -> Resource::self
{
  res_->buffer_info_.usage = static_cast<VkBufferUsageFlagBits>(usage);
  return *this;
}

auto gdm::gfx::Resource<gdm::api::Buffer>::AddMemoryType(gfx::MemoryType memory_type) -> Resource::self
{
  res_->memory_type_ = static_cast<VkMemoryPropertyFlagBits>(memory_type);
  return *this;
}

//--public Buffer

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
