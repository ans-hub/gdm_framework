// *************************************************************
// File:    vk_render_pass.cc (framebuf + renderpass is rend targ)
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_render_pass.h"

#include "render/vk/vk_host_allocator.h"
#include "system/assert_utils.h"
#include "system/array_utils.h"
#include "render/vk/vk_pipeline.h"

#include "render/viewport_desc.h"
#include "render/scissor_desc.h"

// --public AttachmentDescription

gdm::vk::AttachmentDescription::AttachmentDescription(RenderPass& pass, uint index)
  : desc_{ pass.attachment_desc_[index] }
  , ref_{ pass.attachment_ref_[index] }
{ }

// --public RenderPass

gdm::vk::RenderPass::RenderPass(VkDevice device)
  : device_{device}
  , attachment_desc_{}
  , attachment_ref_{}
  , subpass_desc_{}
  , subpass_color_attachments_refs_{}
  , render_pass_{}
  , subpass_dependicies_{}
  , explicitly_finalized_{}
{ }

auto gdm::vk::RenderPass::AddAttachmentDescription(uint idx) -> AttachmentDescription
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  VkAttachmentDescription pass_attachment = { };
  pass_attachment.format = VK_FORMAT_UNDEFINED;
  pass_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  pass_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  pass_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  pass_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  pass_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  pass_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  pass_attachment.finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  arr_utils::EnsureIndex(attachment_desc_, idx);
  attachment_desc_[idx] = pass_attachment;

  VkAttachmentReference attachment_reference = {};
  attachment_reference.attachment = static_cast<uint>(idx);
  attachment_reference.layout = VK_IMAGE_LAYOUT_UNDEFINED;

  arr_utils::EnsureIndex(attachment_ref_, idx);
  attachment_ref_[idx] = attachment_reference;

  return AttachmentDescription(*this, idx);
}

uint gdm::vk::RenderPass::CreateSubpass(gfx::EQueueType type)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  uint next_idx = GetSubpassesCount();
  arr_utils::EnsureIndex(subpass_desc_, next_idx);
  arr_utils::EnsureIndex(subpass_color_attachments_refs_, next_idx);
  
  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;  // todo: type
  
  subpass_desc_[next_idx] = subpass;
  return next_idx;
}

void gdm::vk::RenderPass::AddSubpassColorAttachments(uint subpass_idx, const Attachments& attachments_indices)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  for (uint idx : attachments_indices)
    subpass_color_attachments_refs_[subpass_idx].push_back(attachment_ref_[idx]);

  subpass_desc_[subpass_idx].colorAttachmentCount = static_cast<uint>(attachments_indices.size());
  subpass_desc_[subpass_idx].pColorAttachments = subpass_color_attachments_refs_[subpass_idx].data();
}

void gdm::vk::RenderPass::AddSubpassDepthAttachments(uint subpass_idx, Attachment attachment_idx)
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");
  subpass_desc_[subpass_idx].pDepthStencilAttachment = &attachment_ref_[attachment_idx];
}

void gdm::vk::RenderPass::AddSubpassDependency(uint subpass_idx[2], gfx::EAccess access[2], gfx::EStage stage[2])
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");
  
  subpass_dependicies_.push_back({});
  auto& dep = subpass_dependicies_.back();
  dep.srcSubpass = subpass_idx[0];
  dep.dstSubpass = subpass_idx[1];
  dep.srcAccessMask = static_cast<VkAccessFlagBits>(access[0]);
  dep.dstAccessMask = static_cast<VkAccessFlagBits>(access[1]);
  dep.srcStageMask = static_cast<VkPipelineStageFlagBits>(stage[0]);
  dep.dstStageMask = static_cast<VkPipelineStageFlagBits>(stage[1]);
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
  render_pass_create_info.dependencyCount = static_cast<uint>(subpass_dependicies_.size());
  render_pass_create_info.pDependencies = subpass_dependicies_.data();

  VkResult res = vkCreateRenderPass(device_, &render_pass_create_info, HostAllocator::GetPtr(), &render_pass_);
  ASSERTF(res == VK_SUCCESS, "vkCreateRenderPass error %d", res);

  explicitly_finalized_ = true;
}
