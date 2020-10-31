// *************************************************************
// File:    vk_framebuffer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_FRAMEBUFFER_H
#define GM_VK_FRAMEBUFFER_H

#include "render/defines.h"
#include "render/vk/vk_image.h"
#include "render/vk/vk_render_pass.h"

namespace gdm::vk {

struct FrameBuffer
{
  FrameBuffer(VkDevice device, uint width, uint height, const RenderPass& render_pass, const ImageViews& view);
  operator VkFramebuffer() const { return framebuffer_; }

private:
  auto CreateFrameBuffer(const RenderPass& render_pass, const ImageViews& views) -> VkFramebuffer;

private:
  VkDevice device_;
  uint width_;
  uint height_;
  VkFramebuffer framebuffer_;

}; // struct FrameBuffer

} // namespace gdm::vk

#endif // GM_VK_FRAMEBUFFER_H
