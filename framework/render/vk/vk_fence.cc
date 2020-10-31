// *************************************************************
// File:    vk_fence.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_fence.h"

#include "system/assert_utils.h"

// --public

gdm::vk::Fence::Fence(VkDevice device, gfx::FenceFlags flags)
  : device_{device}
  , fence_{ CreateFence(flags) }
{ }

gdm::vk::Fence::~Fence()
{
  Reset();
}

void gdm::vk::Fence::WaitSignalFromGpu(uint64_t timeout /*=UINT64_MAX*/)
{
  ASSERTF(fence_ != VK_NULL_HANDLE, "WaitSignalFromGpu with null fence");
  VkResult res = vkWaitForFences(device_, 1, &fence_, VK_TRUE, timeout);
  ASSERTF(res == VK_SUCCESS, "vkWaitForFences error %d\n", res);
}

void gdm::vk::Fence::Reset()
{
  VkResult res = vkResetFences(device_, 1, &fence_);
  ASSERTF(res == VK_SUCCESS, "vkResetFences error %d\n", res);
}

// --private

auto gdm::vk::Fence::CreateFence(gfx::FenceFlags /*flags*/) -> VkFence
{
  VkFenceCreateInfo fence_create_info = {};
  fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  VkFence fence;
  VkResult res = vkCreateFence(device_, &fence_create_info, nullptr, &fence);
  ASSERTF(res == VK_SUCCESS, "vkCreateFence error %d\n", res);

  return fence;
}
