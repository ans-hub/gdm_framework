// *************************************************************
// File:    vk_semaphore.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_semaphore.h"

#include "system/assert_utils.h"

// --public

gdm::vk::Semaphore::Semaphore(VkDevice device, gfx::SemaphoreFlags flags)
  : device_{device}
  , semaphore_{ CreateSemaphore(flags) }
{ }

gdm::vk::Semaphore::~Semaphore()
{
  Reset();
}

// --private

auto gdm::vk::Semaphore::CreateSemaphore(gfx::SemaphoreFlags /*flags*/) -> VkSemaphore
{
  VkSemaphoreCreateInfo semaphore_create_info = {};
  semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkSemaphore sem;
  VkResult res = vkCreateSemaphore(device_, &semaphore_create_info, nullptr, &sem);
  ASSERTF(res == VK_SUCCESS, "vkCreateSemaphore error %d\n", res);

  return sem;
}

void gdm::vk::Semaphore::Reset()
{
  vkDestroySemaphore(device_, semaphore_, nullptr);
}
