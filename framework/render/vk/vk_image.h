// *************************************************************
// File:    vk_image.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_H
#define GM_VK_IMAGE_H

#include "render/defines.h"
#include "render/vk/vk_deleter.h"

namespace gdm::vk {

struct Device;

struct Image
{
  using self = Image&;

  Image(Device* device, uint width, uint height);
  ~Image() {}

  Image(const Image&) = delete;
  Image& operator=(const Image&) = delete;

  self GetProps() { return *this; }
  self AddFormatType(gfx::FormatType format) { image_info_.format = VkFormat(format); return *this; }
  self AddImageUsage(gfx::ImageUsage usage) { image_info_.usage = VkImageUsageFlagBits(usage); return *this; }
  void Create();

  uint GetWidth() const { return image_info_.extent.width; }
  uint GetHeight() const { return image_info_.extent.height; }
  uint GetDepth() const { return image_info_.extent.depth; }
  auto GetHandle() const -> VkImage { return image_; }
  auto GetMipsCount() const -> uint32_t { return image_info_.mipLevels; }
  auto GetLayersCount() const -> uint32_t { return image_info_.arrayLayers; }
  auto GetUsage() const -> VkImageUsageFlags { return image_info_.usage;}
  template<class T = VkFormat>
  auto GetFormat() const -> T { return static_cast<T>(image_info_.format); }

  operator VkImage() const { return image_; }

private:
  Device* device_;
  VkImageCreateInfo image_info_;
  VkDeleter<VkImage> image_;
  VkDeleter<VkDeviceMemory> image_memory_;

}; // struct Image

using Image2D = Image;
using Image3D = Image;

namespace helpers
{
  bool HasStencil(VkFormat format);

} // namespace helpers

} // namespace gdm::vk

#endif // GM_VK_IMAGE_H
