// *************************************************************
// File:    vk_host_allocator.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_host_allocator.h"

#include <memory/defines.h>
#include <memory/memory_manager.h>
#include <memory/helpers.h>

// --private

VkAllocationCallbacks gdm::vk::HostAllocator::allocator_ = Initialize();

// --public

void* gdm::vk::HostAllocator::Allocate(void* user_data, size_t size, size_t alignment, VkSystemAllocationScope)
{
  return MemoryManager::AllocateAligned(size, alignment, MEMORY_TAG("Vulkan"));
}

void* gdm::vk::HostAllocator::Reallocate(void* user_data, void* ptr, size_t size, size_t alignment, VkSystemAllocationScope)
{
  return MemoryManager::ReallocateAligned(ptr, size, alignment, MEMORY_TAG("Vulkan"));
}

void gdm::vk::HostAllocator::Free(void* user_data, void* ptr)
{
  return MemoryManager::DeallocateAligned(ptr, 16, MEMORY_TAG("Vulkan"));
}

auto gdm::vk::HostAllocator::Initialize() -> VkAllocationCallbacks
{
  VkAllocationCallbacks allocator;
  allocator.pUserData = nullptr;
  allocator.pfnAllocation = &Allocate;
  allocator.pfnReallocation = &Reallocate;
  allocator.pfnFree = &Free;
  allocator.pfnInternalAllocation = nullptr;
  allocator.pfnInternalFree = nullptr;
  return allocator;
}
