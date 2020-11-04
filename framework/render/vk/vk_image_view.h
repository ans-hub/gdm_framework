// *************************************************************
// File:    vk_image_view.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_IMAGE_VIEW_H
#define GM_VK_IMAGE_VIEW_H

#include <vector>

#include "render/defines.h"

namespace gdm::vk {

struct Device;

struct ImageView
{
  ImageView(VkDevice device, VkImage image, VkFormat format);
  ~ImageView();

  ImageView(const ImageView&) = delete;
  ImageView& operator=(const ImageView&) = delete;

  auto GetFormat() const -> VkFormat { return format_; }
  operator VkImageView() const { return image_view_; }

private:
  auto CreateImageView(VkImage image, VkFormat format) -> VkImageView;

  VkDevice device_;
  VkFormat format_;
  VkImageView image_view_;

}; // struct ImageView

using ImageViews = std::vector<ImageView*>;

} // namespace gdm::vk

#endif // GM_VK_IMAGE_VIEW_H
