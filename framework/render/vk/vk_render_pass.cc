// *************************************************************
// File:    vk_render_pass.cc (framebuf + renderpass is rend targ)
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_render_pass.h"

#include "render/vk/vk_host_allocator.h"
#include "system/assert_utils.h"
#include "render/vk/vk_pipeline.h"

#include "render/viewport_desc.h"
#include "render/scissor_desc.h"

// --public

gdm::vk::RenderPass::RenderPass(VkDevice device)
  : device_{device}
  , descriptior_sets_{}
  , attachment_desc_{}
  , attachment_ref_{}
  , subpass_desc_{}
  , subpass_color_attachments_refs_{}
  , render_pass_{}
  , explicitly_finalized_{}
{ }

void gdm::vk::RenderPass::SetAttachmentDesc(uint idx, gfx::EFormatType format, gfx::EImageLayout layout)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  VkAttachmentDescription pass_attachment = { };
  pass_attachment.format = static_cast<VkFormat>(format);
  pass_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  pass_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  pass_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  pass_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  pass_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  pass_attachment.initialLayout = static_cast<VkImageLayout>(layout);
  pass_attachment.finalLayout = static_cast<VkImageLayout>(layout);

  if (idx >= attachment_desc_.size())
    attachment_desc_.resize(idx + 1);
  attachment_desc_[idx] = pass_attachment;

  VkAttachmentReference attachment_reference = {};
  attachment_reference.attachment = static_cast<uint>(idx);
  attachment_reference.layout = static_cast<VkImageLayout>(layout);
  
  if (idx >= attachment_ref_.size())
    attachment_ref_.resize(idx + 1);
  attachment_ref_[idx] = attachment_reference;
}

void gdm::vk::RenderPass::CreateSubpass(uint idx, gfx::EQueueType type)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;  // todo: type
  
  if (idx >= subpass_desc_.size())
    subpass_desc_.resize(idx + 1);
  subpass_desc_[idx] = subpass;

  if (idx >= subpass_color_attachments_refs_.size())
    subpass_color_attachments_refs_.resize(idx + 1);
}

void gdm::vk::RenderPass::AddColorAttachmentsForSubpass(uint subpass_idx, const Attachments& attachments_indices)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  for (uint idx : attachments_indices)
    subpass_color_attachments_refs_[subpass_idx].push_back(attachment_ref_[idx]);

  subpass_desc_[subpass_idx].colorAttachmentCount = static_cast<uint>(attachments_indices.size());
  subpass_desc_[subpass_idx].pColorAttachments = subpass_color_attachments_refs_[subpass_idx].data();
}

void gdm::vk::RenderPass::AddDepthAttachmentsForSubpass(uint subpass_idx, Attachment attachment_idx)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  subpass_desc_[subpass_idx].pDepthStencilAttachment = &attachment_ref_[attachment_idx];
}

void gdm::vk::RenderPass::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  VkRenderPassCreateInfo render_pass_create_info = {};
  render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_create_info.attachmentCount = static_cast<uint>(attachment_desc_.size());
  render_pass_create_info.pAttachments = attachment_desc_.data();
  render_pass_create_info.subpassCount = static_cast<uint>(subpass_desc_.size());
  render_pass_create_info.pSubpasses = subpass_desc_.data();

  VkResult res = vkCreateRenderPass(device_, &render_pass_create_info, HostAllocator::GetPtr(), &render_pass_);
  ASSERTF(res == VK_SUCCESS, "vkCreateRenderPass error %d", res);

  explicitly_finalized_ = true;
}
