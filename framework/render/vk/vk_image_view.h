// *************************************************************
// File:    vk_image_view.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_VIEW_H
#define GM_VK_IMAGE_VIEW_H

#include <vector>

#include "render/defines.h"
#include "memory/defines.h"

#include "system/assert_utils.h"
#include "render/vk/vk_host_allocator.h"
#include "render/vk/vk_device_allocator.h"
#include "render/vk/vk_device.h"
#include "render/vk/vk_image.h"

#include "vk_deleter.h"

namespace gdm::vk {
  struct ImageView;
}
namespace gdm::gfx {
  template<>
  struct Resource<vk::ImageView>;
}

namespace gdm::vk {

struct ImageView
{
  friend struct gfx::Resource<ImageView>;

  ImageView() =default;
  ImageView(const ImageView&) =delete;
  ImageView& operator=(const ImageView&) =delete;

public:
  auto GetFormat() const -> VkFormat { return image_view_info_.format; }
  auto GetImpl() const -> VkImageView { return image_view_; }
  operator VkImageView() const { return image_view_; }

private:
  VkDevice device_;
  VkImageViewCreateInfo image_view_info_;
  VkDeleter<VkImageView> image_view_; 

}; // struct ImageView

using ImageViews = std::vector<ImageView*>;

} // namespace gdm::vk

namespace gdm::gfx {

template <>
struct Resource<api::ImageView>
{
  using self = Resource<api::ImageView>&;

  Resource(VkDevice device);
  ~Resource();

  self AddImage(VkImage image);
  self AddFormatType(gfx::FormatType format);

  operator api::ImageView*() { return res_; }

private:
  api::ImageView* res_;

}; // struct Resource<api::ImageView>

} // namespace gdm::gfx

#endif // GM_VK_IMAGE_VIEW_H
