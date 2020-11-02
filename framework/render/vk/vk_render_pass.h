// *************************************************************
// File:    vk_render_pass.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_RENDER_PASS_H
#define GM_VK_RENDER_PASS_H

#include <render/defines.h>
#include <render/viewport_desc.h>
#include <render/scissor_desc.h>

#include "render/vk/vk_barrier.h"

namespace gdm::vk{

using Attachments = std::vector<uint>;
using Attachment = uint;

struct RenderPass
{
  RenderPass(VkDevice device);

  void AddPassDesccription(uint attachment_idx, gfx::EFormatType format, gfx::EImageLayout layout);
  uint CreateSubpass(gfx::EQueueType type);
  void AddSubpassColorAttachments(uint subpass_idx, const Attachments& attachments_indices);
  void AddSubpassDepthAttachments(uint subpass_idx, Attachment attachment_index);
  void AddSubpassDependency(uint subpass_idx[2], gfx::EAccess access[2], gfx::EStage stage[2]);
  void Finalize();

  auto GetSubpassesCount() const -> uint { return static_cast<uint>(subpass_desc_.size()); }
  auto GetPassAttachmentsCount() const -> size_t { return attachment_desc_.size(); }
  auto GetHandle() const -> VkRenderPass { return render_pass_; }
  operator VkRenderPass() { return render_pass_; }

private:
  struct Subpass;
  struct Pass;

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
