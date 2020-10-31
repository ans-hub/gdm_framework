// *************************************************************
// File:    vk_image.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_image.h"

#include "system/assert_utils.h"
#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_physical_device.h"

// --public

gdm::vk::Image::Image(Device* device, uint width, uint height, gfx::ImageUsage usage, gfx::FormatType format)
  : device_{ device }
  , image_info_{}
  , image_{VK_NULL_HANDLE}
  , image_memory_{VK_NULL_HANDLE}
{
  image_info_.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info_.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
  image_info_.format = static_cast<VkFormat>(format);
  image_info_.usage = static_cast<VkImageUsageFlagBits>(usage);
  image_info_.imageType = VK_IMAGE_TYPE_2D;
  image_info_.mipLevels = 1;
  image_info_.arrayLayers = 1;
  image_info_.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info_.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info_.queueFamilyIndexCount = 0;
  image_info_.pQueueFamilyIndices = NULL;
  image_info_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  image_ = CreateImage(image_info_);
  image_memory_ = AllocateMemory(image_);
}

gdm::vk::Image::~Image()
{
  if (device_)
  {
    vkDestroyImage(*device_, image_, HostAllocator::GetPtr());
    DeviceAllocator::Free(device_, image_memory_);
  }
}

// --private

auto gdm::vk::Image::CreateImage(const VkImageCreateInfo& info) -> VkImage
{
  VkImage image = {};
  VkResult res = vkCreateImage(*device_, &info, HostAllocator::GetPtr(), &image);
  ASSERTF(res == VK_SUCCESS, "vkCreateImage error %d\n", res);
  return image;
}

auto gdm::vk::Image::AllocateMemory(VkImage image) -> VkDeviceMemory
{
  VkDeviceMemory memory = DeviceAllocator::Allocate(device_, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  VkResult res = vkBindImageMemory(*device_, image, memory, 0);
  ASSERTF(res == VK_SUCCESS, "vkBindBufferMemory error %d\n", res);
  return memory;
}

// --helpers

bool gdm::vk::helpers::HasStencil(VkFormat format)
{
  bool has = false;
  has |= format == VK_FORMAT_D24_UNORM_S8_UINT;
  has |= format == VK_FORMAT_D32_SFLOAT_S8_UINT;
  has |= format == VK_FORMAT_D16_UNORM;
  has |= format == VK_FORMAT_D16_UNORM_S8_UINT;
  return has;
}

auto gdm::vk::helpers::CreateImageView(VkDevice device, VkImage image, VkFormat format) -> VkImageView
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

  VkResult res = vkCreateImageView(device, &image_view_create_info, HostAllocator::GetPtr(), &image_view);
  ASSERTF(res == VK_SUCCESS, "vkCreateImageView error %d\n", res);
  
  return image_view;
}
