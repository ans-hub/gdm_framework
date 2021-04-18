// *************************************************************
// File:    vk_image_barrier.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_image_barrier.h"

#include "render/vk/vk_image.h"
#include "render/vk/vk_image_view.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public Resource<ImageBarrier>

gdm::gfx::Resource<gdm::api::ImageBarrier>::Resource()
  : res_{ GMNew api::ImageBarrier() }
{
  res_->image_barrier_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  res_->image_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  res_->image_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  res_->image_barrier_.subresourceRange.baseMipLevel = 0;
  res_->image_barrier_.subresourceRange.levelCount = 1;
  res_->image_barrier_.subresourceRange.baseArrayLayer = 0;
  res_->image_barrier_.subresourceRange.layerCount = 1;
  res_->image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
}

gdm::gfx::Resource<gdm::api::ImageBarrier>::~Resource()
{
  FillAspectMask(res_->image_barrier_.newLayout, VK_FORMAT_UNDEFINED);
  FillAccessMasks(res_->image_barrier_.oldLayout, res_->image_barrier_.newLayout);
  res_->src_stage_mask_ = api::helpers::GetStageMask(res_->image_barrier_.srcAccessMask);
  res_->dst_stage_mask_ = api::helpers::GetStageMask(res_->image_barrier_.dstAccessMask);
}

auto gdm::gfx::Resource<gdm::api::ImageBarrier>::AddImage(VkImage image) -> Resource::self
{
  res_->image_barrier_.image = image;
  return *this;
}

auto gdm::gfx::Resource<gdm::api::ImageBarrier>::AddOldLayout(gfx::EImageLayout old_layout) -> Resource::self
{
  res_->image_barrier_.oldLayout = VkImageLayout(old_layout);
  return *this;
}

auto gdm::gfx::Resource<gdm::api::ImageBarrier>::AddNewLayout(gfx::EImageLayout new_layout) -> Resource::self
{
  res_->image_barrier_.newLayout = VkImageLayout(new_layout);
  return *this;
}

// --private Resource<ImageBarrier>

void gdm::gfx::Resource<gdm::api::ImageBarrier>::FillAspectMask(VkImageLayout layout, VkFormat format)
{
  if (layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    res_->image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  else
  {
    bool has_stencil = api::helpers::HasStencil(format);
    if (has_stencil)
      res_->image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    else
      res_->image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
}

void gdm::gfx::Resource<gdm::api::ImageBarrier>::FillAccessMasks(VkImageLayout old_layout, VkImageLayout new_layout)
{
  if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
      res_->image_barrier_.srcAccessMask = 0;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = 0;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = 0;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_GENERAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = 0;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = 0;
      res_->image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      res_->image_barrier_.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      res_->image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else
  {
    ASSERTF(false, "Unsupported yet image layout transition");
  }
}

// --helpers

auto gdm::vk::helpers::GetStageMask(VkAccessFlags access) -> VkPipelineStageFlagBits
{
  VkPipelineStageFlagBits mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

  if (bits::HasFlag(access, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT))
    return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  else if (bits::HasFlag(access, VK_ACCESS_UNIFORM_READ_BIT))
    return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT; // TODO:
  else if (bits::HasFlag(access, VK_ACCESS_TRANSFER_WRITE_BIT))
    return VK_PIPELINE_STAGE_TRANSFER_BIT;
  else if (bits::HasFlag(access, VK_ACCESS_TRANSFER_READ_BIT))
    return VK_PIPELINE_STAGE_TRANSFER_BIT;
  else if (bits::HasFlag(access, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT))
    return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  else if (bits::HasFlag(access, VK_ACCESS_SHADER_READ_BIT))
    return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  else
    return mask;
}
