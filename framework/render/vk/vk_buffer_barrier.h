// *************************************************************
// File:    vk_buffer_barrier.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_BUFFER_BARRIER_H
#define GM_VK_BUFFER_BARRIER_H

#include <vector>
#include <memory>

#include "render/defines.h"
#include "render/vk/vk_resource.h"

namespace gdm::vk {
  struct Device;
  struct Buffer;
  struct CommandList;
  struct RenderPass;
  struct BufferBarrier;
} // namespace gdm::vk

namespace gdm::vk {
  template<class T> struct Resource;
  template<> struct Resource<BufferBarrier>;
} // namespace gdm::vk

namespace gdm::vk {

struct BufferBarrier
{
  friend struct Resource<BufferBarrier>;

  BufferBarrier() =default;
  BufferBarrier(const BufferBarrier&) =delete;
  BufferBarrier& operator=(const BufferBarrier&) =delete;

  auto GetImpl() const -> VkBufferMemoryBarrier { return buffer_barrier_; }
  operator VkBufferMemoryBarrier() const { return buffer_barrier_; }

private:
  VkBufferMemoryBarrier buffer_barrier_;
  VkPipelineStageFlagBits src_stage_mask_;
  VkPipelineStageFlagBits dst_stage_mask_;

  friend struct CommandList;
  friend struct RenderPass;

}; // struct BufferBarrier

using BufferBarriers = std::vector<BufferBarrier>;

template <>
struct Resource<BufferBarrier> : public BaseResourceBuilder<BufferBarrier>
{
  Resource(api::Device* device);
  ~Resource() override;

  self AddBuffer(VkBuffer buffer);
  self AddOldAccess(gfx::EAccess old_acces);
  self AddNewAccess(gfx::EAccess new_access);

}; // struct Resource<BufferBarrier>

} // namespace gdm::vk

#endif // GM_VK_BUFFER_BARRIER_H
