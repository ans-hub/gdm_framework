// *************************************************************
// File:    vk_buffer_barrier.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_buffer_barrier.h"

#include "vk_image_barrier.h"

#include "render/vk/vk_image.h"
#include "render/vk/vk_image_view.h"
#include "system/assert_utils.h"
#include "system/bits_utils.h"

// --public Resource<BufferBarrier>

gdm::vk::Resource<gdm::vk::BufferBarrier>::Resource(api::Device* device)
  : BaseResourceBuilder(*device)
{
  ptr_->buffer_barrier_.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
  ptr_->buffer_barrier_.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  ptr_->buffer_barrier_.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  ptr_->buffer_barrier_.size = VK_WHOLE_SIZE;
  ptr_->buffer_barrier_.offset = 0;
}

gdm::vk::Resource<gdm::vk::BufferBarrier>::~Resource()
{
  ptr_->src_stage_mask_ = api::helpers::GetStageMask(ptr_->buffer_barrier_.srcAccessMask);
  ptr_->dst_stage_mask_ = api::helpers::GetStageMask(ptr_->buffer_barrier_.dstAccessMask);
}

auto gdm::vk::Resource<gdm::vk::BufferBarrier>::AddBuffer(VkBuffer buffer) -> Resource::self
{
  ptr_->buffer_barrier_.buffer = buffer;
  return *this;
}

auto gdm::vk::Resource<gdm::vk::BufferBarrier>::AddOldAccess(gfx::EAccess old_access) -> Resource::self
{
  ptr_->buffer_barrier_.srcAccessMask = static_cast<VkAccessFlagBits>(old_access);
  return *this;
}

auto gdm::vk::Resource<gdm::vk::BufferBarrier>::AddNewAccess(gfx::EAccess new_access) -> Resource::self
{
  ptr_->buffer_barrier_.dstAccessMask = static_cast<VkAccessFlagBits>(new_access);
  return *this;
}
