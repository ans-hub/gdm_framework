// *************************************************************
// File:    vk_buffer_barrier.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_BUFFER_BARRIER_H
#define GM_VK_BUFFER_BARRIER_H

#include <vector>

#include "render/defines.h"

namespace gdm::vk {
  struct Device;
  struct Image;
  struct Buffer;
  struct CommandList;
  struct RenderPass;
  struct BufferBarrier;
}
namespace gdm::gfx {
  template<>
  struct Resource<api::BufferBarrier>;
}

namespace gdm::vk {

struct BufferBarrier
{
  friend struct gfx::Resource<BufferBarrier>;

  BufferBarrier() =default;
  BufferBarrier(const BufferBarrier&) =delete;
  BufferBarrier& operator=(const BufferBarrier&) =delete;

  operator VkBufferMemoryBarrier() const { return buffer_barrier_; }

private:
  VkBufferMemoryBarrier buffer_barrier_;
  VkPipelineStageFlagBits src_stage_mask_;
  VkPipelineStageFlagBits dst_stage_mask_;

  friend struct CommandList;
  friend struct RenderPass;

}; // struct BufferBarrier

using BufferBarriers = std::vector<BufferBarrier>;

} // namespace gdm::vk

namespace gdm::gfx {

template <>
struct Resource<api::BufferBarrier>
{
  using self = Resource<api::BufferBarrier>&;

  Resource(api::Device* device);
  ~Resource();

  self AddBuffer(VkBuffer buffer);
  self AddOldAccess(gfx::EAccess old_acces);
  self AddNewAccess(gfx::EAccess new_access);

  operator api::BufferBarrier*() { return res_; }

private:
  api::BufferBarrier* res_;

}; // struct Resource<api::BufferBarrier>

} // namespace gdm::gfx

#endif // GM_VK_BUFFER_BARRIER_H
