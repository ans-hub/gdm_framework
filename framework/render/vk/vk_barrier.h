// *************************************************************
// File:    vk_barrier.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_BARRIERS_H
#define GM_VK_BARRIERS_H

#include <vector>

#include "render/defines.h"

namespace gdm::vk {

struct Device;
struct Image;
struct Buffer;
struct CommandList;
struct RenderPass;

struct ImageBarrier
{
  using self = ImageBarrier&;

  ImageBarrier();

  self GetProps() { return *this; }
  self AddImage(VkImage image) { image_barrier_.image = image; return *this; }
  self AddOldLayout(gfx::EImageLayout old_layout) { image_barrier_.oldLayout = VkImageLayout(old_layout); return *this; }
  self AddNewLayout(gfx::EImageLayout new_layout) { image_barrier_.newLayout = VkImageLayout(new_layout); return *this; }
  void Finalize();

  operator VkImageMemoryBarrier() const { return image_barrier_; }

private:
  void FillAspectMask(VkImageLayout layout, VkFormat format);
  void FillAccessMasks(VkImageLayout old_layout, VkImageLayout new_layout);

private:
  VkImageMemoryBarrier image_barrier_ = {};
  VkPipelineStageFlagBits src_stage_mask_ = {};
  VkPipelineStageFlagBits dst_stage_mask_ = {};

  friend struct CommandList;
  friend struct RenderPass;

}; // struct ImageBarrier

struct BufferBarrier
{
  BufferBarrier(Device* device, VkBuffer buffer, gfx::EAccess old_access, gfx::EAccess new_access);
  operator VkBufferMemoryBarrier() const { return buffer_barrier_; }

private:
  VkBufferMemoryBarrier buffer_barrier_;
  VkPipelineStageFlagBits src_stage_mask_;
  VkPipelineStageFlagBits dst_stage_mask_;

  friend struct CommandList;
  friend struct RenderPass;

}; // struct BufferBarrier

using ImageBarriers = std::vector<ImageBarrier>;
using BufferBarriers = std::vector<BufferBarrier>;

namespace helpers
{
  auto GetStageMask(VkAccessFlags access) -> VkPipelineStageFlagBits;
}

} // namespace gdm::vk

#endif // GM_VK_BARRIERS_H
