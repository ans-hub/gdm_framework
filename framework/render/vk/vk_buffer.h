// *************************************************************
// File:    vk_vertex_buffer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_BUFFER_H
#define GM_VK_BUFFER_H

#include "render/defines.h"

namespace gdm::vk {

struct Device;

struct Buffer
{
  Buffer(Device* device, uint size, gfx::BufferUsage usage, gfx::MemoryType memory_type);
  ~Buffer();

  auto GetSize() const -> uint { return static_cast<uint>(buffer_info_.size); }
  bool IsMapped() const { return mapped_region_ != nullptr; }
  void Map();
  void Map(uint offset, uint size);
  void Unmap();
  template<class T>
  void CopyDataToGpu(const T* data, uint offset, size_t count);

  Buffer(const Buffer& other) = delete;
  Buffer& operator=(const Buffer& other) = delete;
  Buffer(Buffer&& other);
  operator VkBuffer() const { return buffer_; }

private:
  Device* device_;
  VkBufferUsageFlagBits buffer_usage_;
  VkMemoryPropertyFlagBits memory_type_;
  VkBufferCreateInfo buffer_info_;
  VkBuffer buffer_;
  VkDeviceMemory buffer_memory_;
  void* mapped_region_;

}; // struct Buffer

} // namespace gdm::vk

#include "vk_buffer.inl"

#endif // GM_VK_BUFFER_H
