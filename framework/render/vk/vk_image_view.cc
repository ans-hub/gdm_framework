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

gdm::vk::ImageView::ImageView(VkDevice device, VkImage image, VkFormat format)
  : device_{ device }
  , format_{format}
  , image_view_{ CreateImageView(image, format) }
{ }

gdm::vk::ImageView::~ImageView()
{
  vkDestroyImageView(device_, image_view_, HostAllocator::GetPtr());
}

// --private

auto gdm::vk::ImageView::CreateImageView(VkImage image, VkFormat format) -> VkImageView
{
  VkImageViewCreateInfo image_view_create_info = {};
  
  image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  image_view_create_info.image = image;
  image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  image_view_create_info.format = format;
  image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  image_view_create_info.subresourceRange.baseMipLevel = 0;
  image_view_create_info.subresourceRange.levelCount = 1;
  image_view_create_info.subresourceRange.baseArrayLayer = 0;
  image_view_create_info.subresourceRange.layerCount = 1;

  if (helpers::HasStencil(format))
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageView image_view = {};

  VkResult res = vkCreateImageView(device_, &image_view_create_info, HostAllocator::GetPtr(), &image_view);
  ASSERTF(res == VK_SUCCESS, "vkCreateImageView error %d\n", res);
  
  return image_view;
}
