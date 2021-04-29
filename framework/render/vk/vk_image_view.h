// *************************************************************
// File:    vk_image_view.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_VIEW_H
#define GM_VK_IMAGE_VIEW_H

#include <vector>
#include <memory>

#include "render/defines.h"
#include "render/vk/vk_resource.h"
#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_image.h"

#include "memory/defines.h"

#include "system/assert_utils.h"

#include "vk_deleter.h"

namespace gdm::vk {
  struct ImageView;
  struct Device;
}
namespace gdm::vk {
  template<>
  struct Resource<ImageView>;
}

namespace gdm::vk {

struct ImageView
{
  friend struct Resource<ImageView>;

  ImageView() =default;
  ImageView(const ImageView&) =delete;
  ImageView& operator=(const ImageView&) =delete;

public:
  auto GetFormat() const -> VkFormat { return image_view_info_.format; }
  auto GetImpl() const -> VkImageView { return image_view_; }
  operator VkImageView() const { return image_view_; }

private:
  Device* device_;
  VkImageViewCreateInfo image_view_info_;
  VkDeleter<VkImageView> image_view_; 

}; // struct ImageView

using ImageViews = std::vector<ImageView*>;

template <>
struct Resource<ImageView> : BaseResourceBuilder<ImageView>
{
  Resource(vk::Device* device);
  ~Resource() override;

  self AddImage(VkImage image);
  self AddFormatType(gfx::FormatType format);
  self SetName(const char* name);

}; // struct Resource<ImageView>

} // namespace gdm::vk

#endif // GM_VK_IMAGE_VIEW_H
