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

//--public Resource<ImageView>

gdm::gfx::Resource<gdm::api::ImageView>::Resource(VkDevice device)
  : res_{ GMNew api::ImageView() }
{
  res_->device_ = device;
  res_->image_view_info_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  res_->image_view_info_.format = VK_FORMAT_R8G8B8A8_SRGB;
  res_->image_view_info_.image = VK_NULL_HANDLE;
  res_->image_view_info_.viewType = VK_IMAGE_VIEW_TYPE_2D;
  res_->image_view_info_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  res_->image_view_info_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  res_->image_view_info_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  res_->image_view_info_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  res_->image_view_info_.subresourceRange.baseMipLevel = 0;
  res_->image_view_info_.subresourceRange.levelCount = 1;
  res_->image_view_info_.subresourceRange.baseArrayLayer = 0;
  res_->image_view_info_.subresourceRange.layerCount = 1;
}

gdm::gfx::Resource<gdm::api::ImageView>::~Resource()
{
  if (api::helpers::HasStencil(res_->image_view_info_.format))
    res_->image_view_info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    res_->image_view_info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageView image_view = {};
  VkResult res = vkCreateImageView(res_->device_, &res_->image_view_info_, api::HostAllocator::GetPtr(), &image_view);
  ASSERTF(res == VK_SUCCESS, "vkCreateImageView error %d\n", res);

  static auto delete_view_fn = [v = this->res_](VkImageView view) {
    vkDestroyImageView(v->device_, view, api::HostAllocator::GetPtr());
  };

  res_->image_view_ = api::VkDeleter<VkImageView>(delete_view_fn, image_view);  
}

auto gdm::gfx::Resource<gdm::api::ImageView>::AddImage(VkImage image) -> Resource::self
{
  res_->image_view_info_.image = image;
  return *this;
}

auto gdm::gfx::Resource<gdm::api::ImageView>::AddFormatType(gfx::FormatType format) -> Resource::self
{
  res_->image_view_info_.format = VkFormat(format);
  return *this;
}
