// *************************************************************
// File:    vk_image.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_H
#define GM_VK_IMAGE_H

#include <memory>

#include "render/defines.h"
#include "render/vk/vk_resource.h"
#include "render/vk/vk_deleter.h"

namespace gdm::vk {
  struct Image;
  struct Device;
} // namespace gdm::vk

namespace gdm::vk {
  template<class T> struct Resource;
  template<> struct Resource<Image>;
}

namespace gdm::vk {

struct Image
{
  friend struct Resource<Image>;

  Image() =default;
  Image(const Image&) =delete;
  Image& operator=(const Image&) =delete;

public:
  uint GetWidth() const { return image_info_.extent.width; }
  uint GetHeight() const { return image_info_.extent.height; }
  uint GetDepth() const { return image_info_.extent.depth; }
  auto GetHandle() const -> VkImage { return image_; }
  auto GetMipsCount() const -> uint32_t { return image_info_.mipLevels; }
  auto GetLayersCount() const -> uint32_t { return image_info_.arrayLayers; }
  auto GetUsage() const -> VkImageUsageFlags { return image_info_.usage;}
  template<class T = VkFormat>
  auto GetFormat() const -> T { return static_cast<T>(image_info_.format); }

  auto GetImpl() const -> VkImage { return image_; }
  operator VkImage() const { return image_; }

private:
  Device* device_;
  VkImageCreateInfo image_info_;
  VkDeleter<VkImage> image_;
  VkDeleter<VkDeviceMemory> image_memory_;

}; // struct Image

template <>
struct Resource<Image> : public BaseResourceBuilder<Image>
{
  Resource(vk::Device* device);
  ~Resource() override;

  self AddExtent(uint width, uint height, uint depth);
  self AddFormatType(gfx::FormatType format);
  self AddImageUsage(gfx::ImageUsage usage);

}; // struct Resource<Image>

} // namespace gdm::vk

namespace gdm::vk::helpers
{
  bool HasStencil(VkFormat format);

} // namespace gdm::vk::helpers

#endif // GM_VK_IMAGE_H
