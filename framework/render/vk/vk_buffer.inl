// *************************************************************
// File:    vk_buffer.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_buffer.h"

#include "vk_device.h"

#include "memory/helpers.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public

template<class T>
inline void gdm::vk::Buffer::CopyDataToGpu(const T* data, uint offset, size_t count)
{
  ASSERTF(mapped_region_ != nullptr, "Buffer not mapped");
  ASSERTF(bits::HasFlag(memory_type_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT), "Copy is not allowed");

  size_t write_size = count * sizeof(T);
  uintptr_t mapped_begin = mem::PtrToUptr(mapped_region_);
  uintptr_t mapped_end = mapped_begin + buffer_info_.size;
  uintptr_t write_begin = mapped_begin + offset;
  uintptr_t write_end = write_begin + write_size;

  ASSERTF(write_begin >= mapped_begin, "Before range begin %d", (int)(write_begin - mapped_begin));
  ASSERTF(write_end <= mapped_end, "Over range end %d", (int)(write_end - mapped_end));

  memcpy(mem::UptrToPtr(write_begin), data, write_size);

  VkMappedMemoryRange flush_range = {};
  flush_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  flush_range.memory = buffer_memory_;
  flush_range.offset = offset;
  flush_range.size = write_size ;
  
  VkResult res = vkFlushMappedMemoryRanges(*device_, 1, &flush_range);
  ASSERTF(res == VK_SUCCESS, "vkFlushMappedMemoryRanges failed %d", res);
}
