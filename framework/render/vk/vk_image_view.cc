// *************************************************************
// File:    vk_image_view.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_image_view.h"

#include "system/assert_utils.h"
#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_image.h"

// --public

gdm::vk::ImageView::ImageView(VkDevice device)
  : device_{ device }
  , image_view_info_{}
  , image_view_{}
{
  image_view_info_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_info_.format = VK_FORMAT_R8G8B8A8_SRGB;
  image_view_info_.image = VK_NULL_HANDLE;
  image_view_info_.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_info_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_info_.subresourceRange.baseMipLevel = 0;
  image_view_info_.subresourceRange.levelCount = 1;
  image_view_info_.subresourceRange.baseArrayLayer = 0;
  image_view_info_.subresourceRange.layerCount = 1;
}

void gdm::vk::ImageView::Create()
{
  if (helpers::HasStencil(image_view_info_.format))
    image_view_info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    image_view_info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageView image_view = {};
  VkResult res = vkCreateImageView(device_, &image_view_info_, HostAllocator::GetPtr(), &image_view);
  ASSERTF(res == VK_SUCCESS, "vkCreateImageView error %d\n", res);

  static auto destroy_view = [this](VkImageView view) { vkDestroyImageView(device_, view, HostAllocator::GetPtr()); };
  image_view_ = VkDeleter<VkImageView>(destroy_view, image_view);  
}