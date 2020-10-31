// *************************************************************
// File:    vk_buffer.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_buffer.h"

#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public

template<class T>
inline void gdm::vk::Buffer::CopyDataToGpu(const T* data, size_t count)
{
  ASSERTF(mapped_region_ != nullptr, "Buffer not mapped");
  ASSERTF(bits::HasFlag(memory_type_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), "Copy is not allowed");
  ASSERTF(count * sizeof(T) == buffer_info_.size, "Buffer has different size than passed data")

  memcpy(mapped_region_, data, buffer_info_.size);

  VkMappedMemoryRange flush_range = {};
  flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  flush_range.memory = buffer_memory_;
  flush_range.offset = 0;
  flush_range.size = buffer_info_.size;
  
  VkResult res = vkFlushMappedMemoryRanges(*device_, 1, &flush_range);
  ASSERTF(res == VK_SUCCESS, "vkFlushMappedMemoryRanges failed %d", res);
}
