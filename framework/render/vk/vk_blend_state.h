// *************************************************************
// File:    vk_blend_state.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_BLEND_STATE_H
#define GM_VK_BLEND_STATE_H

#include <render/defines.h>

namespace gdm::vk{

using Attachments = std::vector<uint>;
using Attachment = uint;

struct BlendState
{
  struct AttachmentDescription
  {
    using self = AttachmentDescription&;

    AttachmentDescription(BlendState& state, uint index);
    self SetEnabled(bool enabled) { desc_.blendEnable = enabled; return *this; }
    self SetColorWriteMask(gfx::EColorComponent mask) { desc_.colorWriteMask = VkColorComponentFlagBits(mask); return *this; }
    self SetColorWriteMask(gfx::ColorComponent mask) { desc_.colorWriteMask = VkColorComponentFlagBits(mask); return *this; }
    self SetSrcColorBlendFactor(gfx::EBlendFactor factor) { desc_.srcColorBlendFactor = VkBlendFactor(factor); return *this; }
    self SetDstColorBlendFactor(gfx::EBlendFactor factor) { desc_.dstColorBlendFactor = VkBlendFactor(factor); return *this; }
    self SetSrcAlphaBlendFactor(gfx::EBlendFactor factor) { desc_.srcAlphaBlendFactor = VkBlendFactor(factor); return *this; }
    self SetDstAlphaBlendFactor(gfx::EBlendFactor factor) { desc_.dstAlphaBlendFactor = VkBlendFactor(factor); return *this; }
    self SetColorBlendOp(gfx::EBlendOp blend_op) { desc_.colorBlendOp = VkBlendOp(blend_op); return *this; }
    self SetAlphaBlendOp(gfx::EBlendOp blend_op) { desc_.alphaBlendOp = VkBlendOp(blend_op); return *this; }

  private:
    VkPipelineColorBlendAttachmentState& desc_;
  };

  BlendState() = default;
  BlendState(VkDevice device);

  auto AddAttachmentDescription(uint attachment_idx) -> AttachmentDescription;
  auto GetAttachmentsCount() const -> uint { return static_cast<uint>(attachment_desc_.size()); }
  auto GetAttachmentsDescription() const -> const VkPipelineColorBlendAttachmentState* { return attachment_desc_.data(); }
  void Finalize();

private:
  VkDevice device_;
  std::vector<VkPipelineColorBlendAttachmentState> attachment_desc_;
  bool explicitly_finalized_;

}; // struct BlendState

} // namespace gdm::vk

#endif // GM_VK_BLEND_STATE_H
