// *************************************************************
// File:    vk_barrier.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_barrier.h"

#include "render/vk/vk_image.h"
#include "render/vk/vk_image_view.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public ImageBarrier

gdm::vk::ImageBarrier::ImageBarrier()
  : image_barrier_{}
  , src_stage_mask_{}
  , dst_stage_mask_{}  
{
  image_barrier_.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  image_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  image_barrier_.subresourceRange.baseMipLevel = 0;
  image_barrier_.subresourceRange.levelCount = 1;
  image_barrier_.subresourceRange.baseArrayLayer = 0;
  image_barrier_.subresourceRange.layerCount = 1;
  image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
}

void gdm::vk::ImageBarrier::Finalize()
{
  FillAspectMask(image_barrier_.newLayout, VK_FORMAT_UNDEFINED);
  FillAccessMasks(image_barrier_.oldLayout, image_barrier_.newLayout);
  src_stage_mask_ = helpers::GetStageMask(image_barrier_.srcAccessMask);
  dst_stage_mask_ = helpers::GetStageMask(image_barrier_.dstAccessMask);
}

// --public BufferBarrier

gdm::vk::BufferBarrier::BufferBarrier(Device* device, VkBuffer buffer, gfx::EAccess old_access, gfx::EAccess new_access)
  : buffer_barrier_{}
  , src_stage_mask_{}
  , dst_stage_mask_{}
{
  buffer_barrier_.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  buffer_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  buffer_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  buffer_barrier_.buffer = buffer;
  buffer_barrier_.srcAccessMask = static_cast<VkAccessFlagBits>(old_access);
  buffer_barrier_.dstAccessMask = static_cast<VkAccessFlagBits>(new_access);
  buffer_barrier_.size = VK_WHOLE_SIZE;
  buffer_barrier_.offset = 0;
  src_stage_mask_ = helpers::GetStageMask(buffer_barrier_.srcAccessMask);
  dst_stage_mask_ = helpers::GetStageMask(buffer_barrier_.dstAccessMask);
}

// --private ImageBarrier

void gdm::vk::ImageBarrier::FillAspectMask(VkImageLayout layout, VkFormat format)
{
  if (layout != VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  else
  {
    bool has_stencil = helpers::HasStencil(format);
    if (has_stencil)
      image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    else
      image_barrier_.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }
}

void gdm::vk::ImageBarrier::FillAccessMasks(VkImageLayout old_layout, VkImageLayout new_layout)
{
  if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
  {
      image_barrier_.srcAccessMask = 0;
      image_barrier_.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
      image_barrier_.srcAccessMask = 0;
      image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
  {
      image_barrier_.srcAccessMask = 0;
      image_barrier_.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_PREINITIALIZED && new_layout == VK_IMAGE_LAYOUT_GENERAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_GENERAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
      image_barrier_.srcAccessMask = 0;
      image_barrier_.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
    else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
  {
      image_barrier_.srcAccessMask = 0;
      image_barrier_.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  }
  else if (old_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
  {
      image_barrier_.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      image_barrier_.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
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
