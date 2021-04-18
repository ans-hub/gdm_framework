// *************************************************************
// File:    vk_image.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_image.h"

#include "system/assert_utils.h"
#include "memory/defines.h"
#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_physical_device.h"

// --public

gdm::gfx::Resource<gdm::api::Image>::Resource(api::Device* device, uint width, uint height)
  : res_{ GMNew api::Image() }
{
  res_->device_ = device;
  res_->image_info_.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  res_->image_info_.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
  res_->image_info_.format = VK_FORMAT_UNDEFINED;
  res_->image_info_.usage = 0;
  res_->image_info_.imageType = VK_IMAGE_TYPE_2D;
  res_->image_info_.mipLevels = 1;
  res_->image_info_.arrayLayers = 1;
  res_->image_info_.samples = VK_SAMPLE_COUNT_1_BIT;
  res_->image_info_.tiling = VK_IMAGE_TILING_OPTIMAL;
  res_->image_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  res_->image_info_.queueFamilyIndexCount = 0;
  res_->image_info_.pQueueFamilyIndices = NULL;
  res_->image_info_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

gdm::gfx::Resource<gdm::api::Image>::~Resource()
{
  VkImage image = {};
  VkResult res = vkCreateImage(*(res_->device_), &res_->image_info_, api::HostAllocator::GetPtr(), &image);
  ASSERTF(res == VK_SUCCESS, "vkCreateImage error %d\n", res);

  VkDeviceMemory image_memory = api::DeviceAllocator::Allocate(res_->device_, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  res = vkBindImageMemory(*(res_->device_), image, image_memory, 0);
  ASSERTF(res == VK_SUCCESS, "vkBindBufferMemory error %d\n", res);

  static auto destroy_image = [v = this->res_](VkImage image) {
    vkDestroyImage(*(v->device_), image, api::HostAllocator::GetPtr());
  };
  static auto destroy_memory = [v = this->res_](VkDeviceMemory memory) {
    api::DeviceAllocator::Free(v->device_, memory);
  };

  res_->image_ = api::VkDeleter<VkImage>(destroy_image, image);
  res_->image_memory_ = api::VkDeleter<VkDeviceMemory>(destroy_memory, image_memory);
}

auto gdm::gfx::Resource<gdm::api::Image>::AddFormatType(gdm::gfx::FormatType format) -> Resource::self
{
  res_->image_info_.format = VkFormat(format);
  return *this;
}

auto gdm::gfx::Resource<gdm::api::Image>::AddImageUsage(gfx::ImageUsage usage) -> Resource::self
{
  res_->image_info_.usage = VkImageUsageFlagBits(usage);
  return *this;
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
