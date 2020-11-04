// *************************************************************
// File:    vk_framebuffer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_FRAMEBUFFER_H
#define GM_VK_FRAMEBUFFER_H

#include "render/defines.h"
#include "render/vk/vk_image.h"
#include "render/vk/vk_image_view.h"
#include "render/vk/vk_render_pass.h"

namespace gdm::vk {

struct Framebuffer
{
  Framebuffer(VkDevice device, uint width, uint height, const RenderPass& render_pass, ImageViews view);
  uint GetAttachmentsCount() const { return static_cast<uint>(attachment_views_.size()); }
  auto GetAttachmentsView() const -> const ImageViews& { return attachment_views_; }
  auto GetClearValues() const -> const std::vector<VkClearValue>& { return clear_values_; }
  operator VkFramebuffer() const { return framebuffer_; }

private:
  auto CreateClearValues() -> std::vector<VkClearValue>;
  auto CreateFrameBuffer(const RenderPass& render_pass, const ImageViews& views) -> VkFramebuffer;

private:
  VkDevice device_;
  uint width_;
  uint height_;
  ImageViews attachment_views_;
  std::vector<VkClearValue> clear_values_;
  VkFramebuffer framebuffer_;

}; // struct Framebuffer

} // namespace gdm::vk

#endif // GM_VK_FRAMEBUFFER_H
