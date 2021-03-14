// *************************************************************
// File:    vk_sampler.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_sampler.h"

#include "render/vk/vk_host_allocator.h"
#include "system/assert_utils.h"

// --public

gdm::vk::Sampler::Sampler(VkDevice device, const SamplerDesc& sampler_state)
  : device_{ device }
  , sampler_state_{ sampler_state }
  , sampler_{ CreateSampler() }
{ }

gdm::vk::Sampler::~Sampler()
{
  vkDestroySampler(device_, sampler_, HostAllocator::GetPtr());
}

// --private

auto gdm::vk::Sampler::CreateSampler() -> VkSampler
{
  VkSamplerCreateInfo sampler_create_info = {};
  sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  sampler_create_info.flags = 0;
  sampler_create_info.magFilter = VK_FILTER_LINEAR;
  sampler_create_info.minFilter = VK_FILTER_LINEAR;
  sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  sampler_create_info.addressModeU = static_cast<VkSamplerAddressMode>(sampler_state_.address_);
  sampler_create_info.addressModeV = static_cast<VkSamplerAddressMode>(sampler_state_.address_);
  sampler_create_info.addressModeW = static_cast<VkSamplerAddressMode>(sampler_state_.address_);
  sampler_create_info.mipLodBias = 0.f;
  sampler_create_info.anisotropyEnable = VK_FALSE;
  sampler_create_info.maxAnisotropy = 1.f;
  sampler_create_info.compareEnable = VK_FALSE;
  sampler_create_info.compareOp = static_cast<VkCompareOp>(sampler_state_.comparsion_);
  sampler_create_info.minLod = 0.f;
  sampler_create_info.maxLod = 1.f;
  sampler_create_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  sampler_create_info.unnormalizedCoordinates = VK_FALSE;

  VkSampler sampler = {};
  VkResult res = vkCreateSampler(device_, &sampler_create_info, HostAllocator::GetPtr(), &sampler);
  ASSERTF(res == VK_SUCCESS, "vkCreateSampler failed %d", res);
  return sampler;
}
