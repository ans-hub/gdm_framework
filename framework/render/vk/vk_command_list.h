// *************************************************************
// File:    vk_command_list.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_CMD_LIST_H
#define GM_VK_CMD_LIST_H

#include <vector>

#include "math/vector4.h"

#include "vk_defines.h"
#include "vk_image.h"
#include "vk_buffer.h"
#include "vk_barrier.h"
#include "vk_descriptor_set.h"

namespace gdm::vk {

struct Pipeline;
struct Framebuffer;
struct RenderPass;

struct CommandList
{
  CommandList(VkDevice device, VkCommandBuffer buffer, gfx::CommandListFlags flags);
  ~CommandList();

  void Finalize();
  void PushBarrier(const ImageBarrier& image_barrier);
  void PushBarrier(const BufferBarrier& buffer_barrier);
  void CopyBufferToBuffer(Buffer& src, const Buffer& dst, uint src_offset, uint dst_offset, uint size);
  void CopyBufferToBuffer(Buffer& src, const Buffer& dst, uint size);
  void CopyBufferToImage(const Buffer& src, Image& dst, uint size);
  void CopyBufferToImage(const Buffer& src, Image& dst, uint src_offset, uint dst_offset, uint size);
  void BeginRenderPass(const RenderPass& pass, const Framebuffer& framebuffer, uint width, uint height);
  void EndRenderPass();
  void BindPipelineGraphics(VkPipeline pipeline);
  void BindVertexBuffer(VkBuffer vx_buffer);
  void BindIndexBuffer(VkBuffer idx_buffer);
  void BindDescriptorSetGraphics(const vk::DescriptorSets& descriptor_sets, Pipeline& pipeline, const std::vector<uint>& offsets);
  void DrawIndexed(const std::vector<Vec3u>& data);

  bool IsFinalized() const { return explicitly_finalized_; }
  operator VkCommandBuffer();

private:
  auto CreateBeginInfo(gfx::CommandListFlags flags) -> VkCommandBufferBeginInfo;
  void Begin();
  void Reset();

private:
  VkDevice device_;
  VkCommandBuffer command_buffer_;
  VkCommandBufferBeginInfo begin_info_;
  bool explicitly_finalized_;

}; // struct CommandList

using CommandLists = std::vector<VkCommandBuffer>;

} // namespace gdm::vk

#endif // GM_VK_CMD_LIST_H
