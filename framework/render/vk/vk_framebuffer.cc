// *************************************************************
// File:    vk_framebuffer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_framebuffer.h"

#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_image_view.h"
#include "system/assert_utils.h"

// --public

gdm::vk::Framebuffer::Framebuffer(VkDevice device, uint width, uint height, const RenderPass& render_pass, ImageViews views)
  : device_{device}
  , width_{width}
  , height_{height}
  , attachment_views_{ views }
  , clear_values_ { CreateClearValues() }
  , framebuffer_{ CreateFrameBuffer(render_pass, views) }
{ }

// --private

auto gdm::vk::Framebuffer::CreateClearValues() -> std::vector<VkClearValue>
{
  std::vector<VkClearValue> clear_values;
  for (const auto& view : attachment_views_)
  {
    if (helpers::HasStencil(view->GetFormat()))
      clear_values.push_back(VkClearValue{ 1.f, 0.f });
    else
      clear_values.push_back(VkClearValue{ 0.f, 0.f, 0.2f, 1.f });
  }
  return clear_values;
}

auto gdm::vk::Framebuffer::CreateFrameBuffer(const RenderPass& render_pass, const ImageViews& views) -> VkFramebuffer
{
  ASSERTF(views.size() == render_pass.GetPassAttachmentsCount(), "Inconsistent attachments count");
  
  std::vector<VkImageView> vk_views = {};
  for (auto* view : views)
    vk_views.push_back(*view);

  VkFramebufferCreateInfo fb_create_info = {};
  fb_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_create_info.renderPass = render_pass.GetHandle();
  fb_create_info.attachmentCount = static_cast<uint>(vk_views.size());
  fb_create_info.pAttachments = vk_views.data();
  fb_create_info.width = width_;
  fb_create_info.height = height_;
  fb_create_info.layers = 1;

  VkFramebuffer framebuffer;
  VkResult res = vkCreateFramebuffer(device_, &fb_create_info, HostAllocator::GetPtr(), &framebuffer);
  ASSERTF(res == VK_SUCCESS, "vkCreateFramebuffer error %d", res);

  return framebuffer;
}