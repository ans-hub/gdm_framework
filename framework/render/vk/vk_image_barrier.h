// *************************************************************
// File:    vk_image_barrier.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_BARRIER_H
#define GM_VK_IMAGE_BARRIER_H

#include <vector>
#include <memory>

#include "render/defines.h"

namespace gdm::vk {
  struct ImageBarrier;
  struct Device;
  struct Image;
  struct Buffer;
  struct CommandList;
  struct RenderPass;
  struct ImageBarrier;
  struct BufferBarrier;
}
namespace gdm::gfx {
  template<>
  struct Resource<api::ImageBarrier>;
}

namespace gdm::vk {

struct ImageBarrier
{
  friend struct gfx::Resource<ImageBarrier>;

  ImageBarrier() =default;
  ImageBarrier(const ImageBarrier&) =delete;
  ImageBarrier& operator=(const ImageBarrier&) =delete;

  operator VkImageMemoryBarrier() const { return image_barrier_; }

private:
  VkImageMemoryBarrier image_barrier_;
  VkPipelineStageFlagBits src_stage_mask_;
  VkPipelineStageFlagBits dst_stage_mask_;

  friend struct CommandList;
  friend struct RenderPass;

}; // struct ImageBarrier

using ImageBarriers = std::vector<ImageBarrier*>;

} // namespace gdm::vk

namespace gdm::gfx {

template <>
struct Resource<api::ImageBarrier>
{
  using self = Resource<api::ImageBarrier>&;

  Resource();
  ~Resource();

  self AddImage(VkImage image);
  self AddOldLayout(gfx::EImageLayout old_layout);
  self AddNewLayout(gfx::EImageLayout new_layout);

  operator api::ImageBarrier*() { return res_; }
  operator std::unique_ptr<api::ImageBarrier>() { return std::unique_ptr<api::ImageBarrier>(res_); }

private:
  void FillAspectMask(VkImageLayout layout, VkFormat format);
  void FillAccessMasks(VkImageLayout old_layout, VkImageLayout new_layout);

private:
  api::ImageBarrier* res_;

}; // struct Resource<api::ImageBarrier>

} // namespace gdm::gfx

namespace gdm::vk::helpers {

  auto GetStageMask(VkAccessFlags access) -> VkPipelineStageFlagBits;

} // namespace gdm::vk

#endif // GM_VK_IMAGE_BARRIER_H
