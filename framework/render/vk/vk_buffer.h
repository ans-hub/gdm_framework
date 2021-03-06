// *************************************************************
// File:    vk_vertex_buffer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_BUFFER_H
#define GM_VK_BUFFER_H

#include "render/defines.h"
#include "render/vk/vk_resource.h"

#include <memory>

namespace gdm::vk {
  struct Device;
  struct Buffer;
} // namespace gdm::vk

namespace gdm::vk {
  template<class T> struct Resource;
  template<> struct Resource<Buffer>;
} // namespace gdm::vk

namespace gdm::vk {

struct Buffer
{
  friend struct Resource<Buffer>;

  Buffer() =default;
  ~Buffer();

  Buffer(const Buffer& other) =delete;
  Buffer& operator=(const Buffer& other) =delete;
  Buffer(Buffer&& other);

  auto GetSize() const -> uint { return static_cast<uint>(buffer_info_.size); }
  bool IsMapped() const { return mapped_region_ != nullptr; }
  void Map();
  void Map(uint offset, uint size);
  void Unmap();

  template<class T>
  void CopyDataToGpu(const T* data, uint offset, size_t count);

  void CopyDataToGpu(const void* data, uint offset, size_t write_size);

  auto GetImpl() const -> VkBuffer { return buffer_; }
  operator VkBuffer() const { return buffer_; }

private:
  Device* device_;
  VkMemoryPropertyFlagBits memory_type_;
  VkBufferCreateInfo buffer_info_;
  VkBuffer buffer_;
  VkDeviceMemory buffer_memory_;
  void* mapped_region_;
  VkDeviceSize flush_range_alignment_;

}; // struct Buffer

template <>
struct Resource<Buffer> : public BaseResourceBuilder<Buffer>
{
  Resource(api::Device* device, uint size);
  ~Resource();

  self AddUsage(gfx::BufferUsage usage);
  self AddMemoryType(gfx::MemoryType memory_type);

}; // struct Resource<api::Buffer>

} // namespace gdm::vk

#include "vk_buffer.inl"

#endif // GM_VK_BUFFER_H
