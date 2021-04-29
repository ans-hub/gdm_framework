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
#include "render/vk/vk_resource.h"

namespace gdm::vk {
  struct ImageBarrier;
  struct Device;
  struct Image;
  struct Buffer;
  struct CommandList;
  struct RenderPass;
  struct ImageBarrier;
}
namespace gdm::vk {
  template<class T> struct Resource;
  template<> struct Resource<ImageBarrier>;
}

namespace gdm::vk {

struct ImageBarrier
{
  friend struct Resource<ImageBarrier>;

  ImageBarrier() =default;
  ImageBarrier(const ImageBarrier&) =delete;
  ImageBarrier& operator=(const ImageBarrier&) =delete;

  auto GetImpl() const -> VkImageMemoryBarrier { return image_barrier_; }
  operator VkImageMemoryBarrier() const { return image_barrier_; }

private:
  VkImageMemoryBarrier image_barrier_;
  VkPipelineStageFlagBits src_stage_mask_;
  VkPipelineStageFlagBits dst_stage_mask_;

  friend struct CommandList;
  friend struct RenderPass;

}; // struct ImageBarrier

using ImageBarriers = std::vector<ImageBarrier*>;

template <>
struct Resource<ImageBarrier> : public BaseResourceBuilder<ImageBarrier>
{
  Resource(api::Device* device);
  ~Resource() override;

  self AddImage(VkImage image);
  self AddOldLayout(gfx::EImageLayout old_layout);
  self AddNewLayout(gfx::EImageLayout new_layout);

private:
  void FillAspectMask(VkImageLayout layout, VkFormat format);
  void FillAccessMasks(VkImageLayout old_layout, VkImageLayout new_layout);

}; // struct Resource<api::ImageBarrier>

} // namespace gdm::gfx

namespace gdm::vk::helpers {

  auto GetStageMask(VkAccessFlags access) -> VkPipelineStageFlagBits;

} // namespace gdm::vk

#endif // GM_VK_IMAGE_BARRIER_H
