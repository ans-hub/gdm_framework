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
#include "render/vk/vk_renderer.h"
#include "render/vk/vk_image.h"

//--public

gdm::vk::Resource<gdm::vk::ImageView>::Resource(vk::Device* device)
  : BaseResourceBuilder(*device)
{
  ptr_->device_ = device;
  ptr_->image_view_info_.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ptr_->image_view_info_.format = VK_FORMAT_R8G8B8A8_SRGB;
  ptr_->image_view_info_.image = VK_NULL_HANDLE;
  ptr_->image_view_info_.viewType = VK_IMAGE_VIEW_TYPE_2D;
  ptr_->image_view_info_.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  ptr_->image_view_info_.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  ptr_->image_view_info_.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  ptr_->image_view_info_.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  ptr_->image_view_info_.subresourceRange.baseMipLevel = 0;
  ptr_->image_view_info_.subresourceRange.levelCount = 1;
  ptr_->image_view_info_.subresourceRange.baseArrayLayer = 0;
  ptr_->image_view_info_.subresourceRange.layerCount = 1;
}

gdm::vk::Resource<gdm::vk::ImageView>::~Resource()
{
  if (vk::helpers::HasStencil(ptr_->image_view_info_.format))
    ptr_->image_view_info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    ptr_->image_view_info_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageView image_view = {};
  VkResult res = vkCreateImageView(*ptr_->device_, &ptr_->image_view_info_, api::HostAllocator::GetPtr(), &image_view);
  ASSERTF(res == VK_SUCCESS, "vkCreateImageView error %d\n", res);

  static auto delete_view_fn = [v = this->ptr_](VkImageView view) {
    vkDestroyImageView(*v->device_, view, api::HostAllocator::GetPtr());
  };

  ptr_->image_view_ = api::VkDeleter<VkImageView>(delete_view_fn, image_view);
}

auto gdm::vk::Resource<gdm::vk::ImageView>::AddImage(VkImage image) -> Resource::self
{
  ptr_->image_view_info_.image = image;
  return *this;
}

auto gdm::vk::Resource<gdm::vk::ImageView>::AddFormatType(gfx::FormatType format) -> Resource::self
{
  ptr_->image_view_info_.format = VkFormat(format);
  return *this;
}

auto gdm::vk::Resource<gdm::vk::ImageView>::SetName(const char* name) -> Resource::self
{
  name_ = name;
  return *this;
}
