// *************************************************************
// File:    vk_command_list.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_command_list.h"

#include <render/colors.h>

#include <render/vk/vk_image.h>
#include <render/vk/vk_image_view.h>
#include <render/vk/vk_pipeline.h>
#include <render/vk/vk_descriptor_set.h>
#include <render/vk/vk_framebuffer.h>
#include <render/vk/vk_render_pass.h>

#include <system/assert_utils.h>
#include <system/bits_utils.h>

// --public

gdm::vk::CommandList::CommandList(VkDevice device, VkCommandBuffer buffer, gfx::CommandListFlags flags)
  : device_{ device }
  , command_buffer_{ buffer }
  , begin_info_{ CreateBeginInfo(flags) }
  , explicitly_finalized_{ false }
{
  Begin();
}

gdm::vk::CommandList::~CommandList()
{
  if (!explicitly_finalized_)
    Finalize();
  Reset();
}

void gdm::vk::CommandList::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");

  VkResult res = vkEndCommandBuffer(command_buffer_);
  ASSERTF(res == VK_SUCCESS, "vkEndCommandBuffer %d", res);
  explicitly_finalized_ = true;
}

void gdm::vk::CommandList::PushBarrier(const ImageBarrier& barrier)
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");

  const VkImageMemoryBarrier& vk_barrier = barrier;
  const VkPipelineStageFlagBits src = barrier.src_stage_mask_;
  const VkPipelineStageFlagBits dst = barrier.dst_stage_mask_;

  vkCmdPipelineBarrier(command_buffer_, src, dst, 0, 0, NULL, 0, NULL, 1, &vk_barrier);
}

void gdm::vk::CommandList::PushBarrier(const BufferBarrier& barrier)
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");

  const VkBufferMemoryBarrier vk_barrier = barrier;
  const VkPipelineStageFlagBits src = barrier.src_stage_mask_;
  const VkPipelineStageFlagBits dst = barrier.dst_stage_mask_;

  vkCmdPipelineBarrier(command_buffer_, src, dst, 0, 0, NULL, 1, &vk_barrier, 0, NULL);
}

void gdm::vk::CommandList::CopyBufferToBuffer(Buffer& src, const Buffer& dst, uint size)
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");

  VkBufferCopy copy_region = {};
  copy_region.srcOffset = 0;
  copy_region.dstOffset = 0;
  copy_region.size = size;

  vkCmdCopyBuffer(command_buffer_, src, dst, 1, &copy_region);
}

void gdm::vk::CommandList::CopyBufferToBuffer(Buffer& src, const Buffer& dst, uint src_offset, uint dst_offset, uint size)
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");

  VkBufferCopy copy_region = {};
  copy_region.srcOffset = src_offset;
  copy_region.dstOffset = dst_offset;
  copy_region.size = size;

  vkCmdCopyBuffer(command_buffer_, src, dst, 1, &copy_region);
}

void gdm::vk::CommandList::CopyBufferToImage(const Buffer& src, Image& dst, uint size)
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");
  ASSERTF(bits::HasFlag(dst.GetUsage(), VK_IMAGE_USAGE_TRANSFER_DST_BIT), "Incorrec dst image");

  VkBufferImageCopy copy_image = {};
  copy_image.imageExtent.width = dst.GetWidth();
  copy_image.imageExtent.height = dst.GetHeight();
  copy_image.imageExtent.depth = 1;
  copy_image.imageSubresource.layerCount = 1;

  if (helpers::HasStencil(dst.GetFormat<VkFormat>()))
    copy_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    copy_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  vkCmdCopyBufferToImage(command_buffer_, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_image);
}

void gdm::vk::CommandList::CopyBufferToImage(const Buffer& src, Image& dst, uint src_offset, uint dst_offset, uint size)
{
  ASSERTF(dst_offset == 0, "Dst offset for  buffers not implemented");
  ASSERTF(!explicitly_finalized_, "Command list finalized");
  ASSERTF(bits::HasFlag(dst.GetUsage(), VK_IMAGE_USAGE_TRANSFER_DST_BIT), "Incorrec dst image");

  VkBufferImageCopy copy_image = {};
  copy_image.imageExtent.width = dst.GetWidth();
  copy_image.imageExtent.height = dst.GetHeight();
  copy_image.imageExtent.depth = 1;
  copy_image.imageSubresource.layerCount = 1;
  copy_image.bufferOffset = src_offset;

  if (helpers::HasStencil(dst.GetFormat<VkFormat>()))
    copy_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  else
    copy_image.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

  vkCmdCopyBufferToImage(command_buffer_, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_image);
}

void gdm::vk::CommandList::BeginRenderPass(const RenderPass& pass, const Framebuffer& framebuffer, uint width, uint height)
{
  ASSERTF(!explicitly_finalized_, "Command list finalized");

  VkRenderPassBeginInfo begin_info = {};
  begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  begin_info.renderPass = pass;
  begin_info.framebuffer = framebuffer;
  begin_info.renderArea = { 0, 0, width, height };

  auto& clear_values = framebuffer.GetClearValues();
  begin_info.clearValueCount = static_cast<uint>(clear_values.size());
  begin_info.pClearValues = clear_values.data();

  vkCmdBeginRenderPass(command_buffer_, &begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void gdm::vk::CommandList::EndRenderPass()
{
  vkCmdEndRenderPass(command_buffer_);
}

void gdm::vk::CommandList::BindPipelineGraphics(VkPipeline pipeline)
{
  vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void gdm::vk::CommandList::BindVertexBuffer(VkBuffer vx_buffer)
{
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(command_buffer_, 0, 1, &vx_buffer, offsets);
}

void gdm::vk::CommandList::BindIndexBuffer(VkBuffer idx_buffer)
{
  vkCmdBindIndexBuffer(command_buffer_, idx_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void gdm::vk::CommandList::BindDescriptorSetGraphics(const vk::DescriptorSets& descriptor_sets, Pipeline& pipeline, const std::vector<uint>& offsets)
{
  std::vector<VkDescriptorSet> descriptor_set_data;
  for (auto& set : descriptor_sets)
    descriptor_set_data.push_back(set.get());
  
  uint descriptor_set_cnt = static_cast<uint>(descriptor_sets.size());
  uint offset_cnt = static_cast<uint>(offsets.size());

  VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
  vkCmdBindDescriptorSets(command_buffer_, bind_point, pipeline, 0, descriptor_set_cnt, descriptor_set_data.data(), offset_cnt, offsets.data());
}

void gdm::vk::CommandList::DrawIndexed(const std::vector<Vec3u>& data)
{
  vkCmdDrawIndexed(command_buffer_, static_cast<uint32_t>(data.size() * 3), 1, 0, 0, 0);
}

gdm::vk::CommandList::operator VkCommandBuffer()
{
  ASSERTF(explicitly_finalized_, "Trying to access underlying data while command list not finalized");
  return command_buffer_;
}

// --private

auto gdm::vk::CommandList::CreateBeginInfo(gfx::CommandListFlags flags) -> VkCommandBufferBeginInfo
{
  VkCommandBufferBeginInfo begin_info {};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  begin_info.flags = static_cast<VkCommandBufferUsageFlags>(flags);
  return begin_info;
}  

void gdm::vk::CommandList::Begin()
{
  VkResult res = vkBeginCommandBuffer(command_buffer_, &begin_info_);
  ASSERTF(res == VK_SUCCESS, "vkBeginCommandBuffer %d", res);
}

void gdm::vk::CommandList::Reset()
{
  vkResetCommandBuffer(command_buffer_, 0);
}
