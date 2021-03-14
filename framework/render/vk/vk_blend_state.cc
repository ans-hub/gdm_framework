// *************************************************************
// File:    vk_blend_state.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_blend_state.h"

#include "render/vk/vk_host_allocator.h"
#include "system/assert_utils.h"
#include "system/array_utils.h"
#include "render/vk/vk_pipeline.h"

#include "render/desc/viewport_desc.h"
#include "render/desc/scissor_desc.h"

// --public

gdm::vk::BlendState::AttachmentDescription::AttachmentDescription(BlendState& state, uint index)
  : desc_{ state.attachment_desc_[index] }
{ }

// --public RenderPass

gdm::vk::BlendState::BlendState(VkDevice device)
  : device_{device}
  , explicitly_finalized_{false}
{ }

auto gdm::vk::BlendState::AddAttachmentDescription(uint idx) -> BlendState::AttachmentDescription
{
  ASSERTF(!explicitly_finalized_, "Render pass finalized");

  VkPipelineColorBlendAttachmentState attachment = { };
  attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  attachment.blendEnable = VK_FALSE;

  arr_utils::EnsureIndex(attachment_desc_, idx);
  attachment_desc_[idx] = attachment;

  return AttachmentDescription(*this, idx);
}

void gdm::vk::BlendState::Finalize()
{
  ASSERTF(!explicitly_finalized_, "Blend state already finalized");

  explicitly_finalized_ = true;
}
