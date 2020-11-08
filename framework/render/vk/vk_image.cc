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

gdm::vk::Image::Image(Device* device, uint width, uint height)
  : device_{ device }
  , image_info_{}
  , image_{}
  , image_memory_{}
{
  image_info_.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info_.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
  image_info_.format = VK_FORMAT_UNDEFINED;
  image_info_.usage = 0;
  image_info_.imageType = VK_IMAGE_TYPE_2D;
  image_info_.mipLevels = 1;
  image_info_.arrayLayers = 1;
  image_info_.samples = VK_SAMPLE_COUNT_1_BIT;
  image_info_.tiling = VK_IMAGE_TILING_OPTIMAL;
  image_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info_.queueFamilyIndexCount = 0;
  image_info_.pQueueFamilyIndices = NULL;
  image_info_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void gdm::vk::Image::Create()
{
  static auto destroy_image = [this](VkImage image) { vkDestroyImage(*device_, image, HostAllocator::GetPtr()); };
  static auto destroy_memory = [this](VkDeviceMemory memory) { DeviceAllocator::Free(device_, memory); };
  
  VkImage image = {};
  VkResult res = vkCreateImage(*device_, &image_info_, HostAllocator::GetPtr(), &image);
  ASSERTF(res == VK_SUCCESS, "vkCreateImage error %d\n", res);

  VkDeviceMemory image_memory = DeviceAllocator::Allocate(device_, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  res = vkBindImageMemory(*device_, image, image_memory, 0);
  ASSERTF(res == VK_SUCCESS, "vkBindBufferMemory error %d\n", res);

  image_ = VkDeleter<VkImage>(destroy_image, image);
  image_memory_ = VkDeleter<VkDeviceMemory>(destroy_memory, image_memory);
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
