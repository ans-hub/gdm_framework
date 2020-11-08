// *************************************************************
// File:    vk_image_view.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_VIEW_H
#define GM_VK_IMAGE_VIEW_H

#include <vector>

#include "render/defines.h"
#include "vk_deleter.h"

namespace gdm::vk {

struct Device;

struct ImageView
{
  using self = ImageView&;

  ImageView(VkDevice device);
 
  ImageView(const ImageView&) = delete;
  ImageView& operator=(const ImageView&) = delete;

  self GetProps() { return *this; }
  self AddImage(VkImage image) { image_view_info_.image = image; return *this; }
  self AddFormatType(gfx::FormatType format) { image_view_info_.format = VkFormat(format); return *this; }
  void Create();

  auto GetFormat() const -> VkFormat { return image_view_info_.format; }
  operator VkImageView() const { return image_view_; }

private:
  VkDevice device_;
  VkImageViewCreateInfo image_view_info_;
  VkDeleter<VkImageView> image_view_;

}; // struct ImageView

using ImageViews = std::vector<ImageView*>;

} // namespace gdm::vk

#endif // GM_VK_IMAGE_VIEW_H
