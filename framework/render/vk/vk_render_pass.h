// *************************************************************
// File:    vk_render_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_RENDER_PASS_H
#define GM_VK_RENDER_PASS_H

#include <render/defines.h>
#include <render/desc/viewport_desc.h>
#include <render/desc/scissor_desc.h>

#include "render/vk/vk_barrier.h"

namespace gdm::vk{

using Attachments = std::vector<uint>;
using Attachment = uint;

struct AttachmentDescription
{
  using self = AttachmentDescription&;

  AttachmentDescription(RenderPass& pass, uint index);
  self AddFormat(gfx::EFormatType format) { desc_.format = VkFormat(format); return *this; }
  self AddInitLayout(gfx::EImageLayout layout) { desc_.initialLayout = VkImageLayout(layout); return *this; }
  self AddFinalLayout(gfx::EImageLayout layout) { desc_.finalLayout = VkImageLayout(layout); return *this; }
  self AddRefLayout(gfx::EImageLayout layout) { ref_.layout = VkImageLayout(layout); return *this; }
  self AddLoadOp(gfx::EAttachmentLoadOp load_op) { desc_.loadOp = VkAttachmentLoadOp(load_op); return *this; }
  self AddStoreOp(gfx::EAttachmentStoreOp store_op) { desc_.storeOp = VkAttachmentStoreOp(store_op); return *this; }

private:
  VkAttachmentDescription& desc_;
  VkAttachmentReference& ref_;
};

struct RenderPass
{
  RenderPass(VkDevice device);

  auto AddAttachmentDescription(uint attachment_idx) -> AttachmentDescription;
  uint CreateSubpass(gfx::EQueueType type);
  void AddSubpassColorAttachments(uint subpass_idx, const Attachments& attachments_indices);
  void AddSubpassDepthAttachments(uint subpass_idx, Attachment attachment_index);
  void AddSubpassDependency(uint subpass_idx[2], gfx::EAccess access[2], gfx::EStage stage[2]);
  void Finalize();

  auto GetSubpassesCount() const -> uint { return static_cast<uint>(subpass_desc_.size()); }
  auto GetPassAttachmentsCount() const -> uint { return static_cast<uint>(attachment_desc_.size()); }
  auto GetHandle() const -> VkRenderPass { return render_pass_; }

  operator VkRenderPass() const { return render_pass_; }

private:
  struct Subpass;
  struct Pass;
  friend struct AttachmentDescription;

private:
  VkDevice device_;
  // PassInfo { attdesc, attref }
  std::vector<VkAttachmentDescription> attachment_desc_;          // overall
  std::vector<VkAttachmentReference> attachment_ref_;             // overall
  // std::vector<Subpass>
  std::vector<VkSubpassDescription> subpass_desc_;                // per pass
  std::vector<VkSubpassDependency> subpass_dependicies_;          // per pass
  std::vector<std::vector<VkAttachmentReference>> subpass_color_attachments_refs_;  // per pass
  
  VkRenderPass render_pass_;
  bool explicitly_finalized_;

}; // struct RenderPass

} // namespace gdm::vk

#endif // GM_VK_RENDER_PASS_H
