// *************************************************************
// File:    vk_image.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_image.h"

#include "system/assert_utils.h"
#include "memory/defines.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_physical_device.h"
#include "render/vk/vk_device_allocator.h"
#include "render/vk/vk_host_allocator.h"

// --public

gdm::vk::Resource<gdm::vk::Image>::Resource(vk::Device* device)
  : BaseResourceBuilder(*device)
{
  ptr_->device_ = device;
  ptr_->image_info_.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ptr_->image_info_.extent = {};
  ptr_->image_info_.format = VK_FORMAT_UNDEFINED;
  ptr_->image_info_.usage = 0;
  ptr_->image_info_.imageType = VK_IMAGE_TYPE_2D;
  ptr_->image_info_.mipLevels = 1;
  ptr_->image_info_.arrayLayers = 1;
  ptr_->image_info_.samples = VK_SAMPLE_COUNT_1_BIT;
  ptr_->image_info_.tiling = VK_IMAGE_TILING_OPTIMAL;
  ptr_->image_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ptr_->image_info_.queueFamilyIndexCount = 0;
  ptr_->image_info_.pQueueFamilyIndices = NULL;
  ptr_->image_info_.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
}

gdm::vk::Resource<gdm::vk::Image>::~Resource()
{
  VkImage image = {};
  VkResult res = vkCreateImage(*(ptr_->device_), &ptr_->image_info_, vk::HostAllocator::GetPtr(), &image);
  ASSERTF(res == VK_SUCCESS, "vkCreateImage error %d\n", res);

  VkDeviceMemory image_memory = vk::DeviceAllocator::Allocate(ptr_->device_, image, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  res = vkBindImageMemory(*(ptr_->device_), image, image_memory, 0);
  ASSERTF(res == VK_SUCCESS, "vkBindBufferMemory error %d\n", res);

  static auto destroy_image = [v = this->ptr_](VkImage image) {
    vkDestroyImage(*(v->device_), image, vk::HostAllocator::GetPtr());
  };
  static auto destroy_memory = [v = this->ptr_](VkDeviceMemory memory) {
    vk::DeviceAllocator::Free(v->device_, memory);
  };

  ptr_->image_ = vk::VkDeleter<VkImage>(destroy_image, image);
  ptr_->image_memory_ = vk::VkDeleter<VkDeviceMemory>(destroy_memory, image_memory);
}

auto gdm::vk::Resource<gdm::vk::Image>::AddExtent(uint width, uint height, uint depth) -> Resource::self
{
  ptr_->image_info_.extent = {
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height),
    static_cast<uint32_t>(depth)
  };
  return *this;
}

auto gdm::vk::Resource<gdm::vk::Image>::AddFormatType(gdm::gfx::FormatType format) -> Resource::self
{
  ptr_->image_info_.format = VkFormat(format);
  return *this;
}

auto gdm::vk::Resource<gdm::vk::Image>::AddImageUsage(gfx::ImageUsage usage) -> Resource::self
{
  ptr_->image_info_.usage = VkImageUsageFlagBits(usage);
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
