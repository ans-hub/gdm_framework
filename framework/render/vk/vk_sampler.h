// *************************************************************
// File:    vk_sampler.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_SAMPLER_H
#define GM_VK_SAMPLER_H

#include "render/defines.h"
#include "render/desc/sampler_desc.h"

namespace gdm::vk {

struct Sampler
{
  Sampler(VkDevice device, const SamplerDesc& sampler);
  ~Sampler();
  operator VkSampler() const { return sampler_; }

private:
  auto CreateSampler() -> VkSampler;

private:
  VkDevice device_;
  SamplerDesc sampler_state_;
  VkSampler sampler_;

}; // struct Sampler

} // namespace gdm::vk

#endif // GM_VK_SAMPLER_H
