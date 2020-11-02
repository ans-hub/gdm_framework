// *************************************************************
// File:    vk_framebuffer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_framebuffer.h"

#include "render/vk/vk_host_allocator.h"
#include "system/assert_utils.h"

// --public

gdm::vk::FrameBuffer::FrameBuffer(VkDevice device, uint width, uint height, const RenderPass& render_pass, const ImageViews& views)
  : device_{device}
  , width_{width}
  , height_{height}
  , framebuffer_{ CreateFrameBuffer(render_pass, views) }
{ }

auto gdm::vk::FrameBuffer::CreateFrameBuffer(const RenderPass& render_pass, const ImageViews& views) -> VkFramebuffer
{
  ASSERTF(views.size() == render_pass.GetPassAttachmentsCount(), "Inconsistent attachments count");

  VkFramebufferCreateInfo fb_create_info = {};
  fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_create_info.renderPass = render_pass.GetHandle();
  fb_create_info.attachmentCount = static_cast<uint>(views.size());
  fb_create_info.pAttachments = views.data();
  fb_create_info.width = width_;
  fb_create_info.height = height_;
  fb_create_info.layers = 1;

  VkFramebuffer framebuffer;
  VkResult res = vkCreateFramebuffer(device_, &fb_create_info, HostAllocator::GetPtr(), &framebuffer);
  ASSERTF(res == VK_SUCCESS, "vkCreateFramebuffer error %d", res);

  return framebuffer;
}