// *************************************************************
// File:    vk_device.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_device.h"

#include <vector>

#include "render/defines.h"
#include "render/vk/vk_host_allocator.h"
#include "system/assert_utils.h"

gdm::vk::Device::Device(const PhysicalDeviceId& phys_device)
  : allocator_{ *HostAllocator::GetPtr() }
  , phys_device_{ phys_device }
  , device_extensions_{ FillDeviceExtensionsInfo() }
  , device_{ CreateLogicalDevice(phys_device_.info_.device_, phys_device.queue_id) }
  , graphics_queue_{ CreateQueue(phys_device.queue_id) }
  , presentation_queue_{ CreateQueue(phys_device.queue_id) }
{ }

gdm::vk::Device::~Device()
{
  vkDestroyDevice(device_, &allocator_);
}

auto gdm::vk::Device::GetQueue(gfx::QueueType type) -> VkQueue
{
  switch(type)
  {
    case gfx::GRAPHICS : return graphics_queue_;
    case gfx::PRESENTATION : return presentation_queue_;
    default : ASSERTF(false, "Queue doesn't implemented");
  }
  return {};
}

// --private

auto gdm::vk::Device::FillDeviceExtensionsInfo() const -> std::vector<const char*>
{
  std::vector<const char*> device_extensions;
  
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  device_extensions.push_back(VK_KHR_MAINTENANCE3_EXTENSION_NAME);
  device_extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
 
  ASSERT(helpers::ValidateDeviceExtensionsProps(*this, device_extensions).empty());

  return device_extensions;
}

auto gdm::vk::Device::CreateLogicalDevice(VkPhysicalDevice phys_device, int queue_family_idx) -> VkDevice
{
  float queue_priorities = 1.0f;

  VkDeviceQueueCreateInfo queue_info = {};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = queue_family_idx;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = &queue_priorities;
    
  VkPhysicalDeviceVulkan12Features device_features12 = {};
  device_features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  device_features12.timelineSemaphore = true;
  device_features12.descriptorBindingSampledImageUpdateAfterBind = true;
  device_features12.shaderSampledImageArrayNonUniformIndexing = true;
  device_features12.runtimeDescriptorArray = true;
  device_features12.descriptorBindingVariableDescriptorCount = true;
  device_features12.descriptorBindingPartiallyBound = true;
  device_features12.descriptorIndexing = true;
  device_features12.pNext = NULL;
  
  VkPhysicalDeviceFeatures device_features = {};
  device_features.shaderClipDistance = true;
#ifndef _NDEBUG
  device_features.fillModeNonSolid = true;
#else
  device_features.fillModeNonSolid = false;
#endif

  // todo: validate device features using info_.device_features_

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions_.size());
  device_info.ppEnabledExtensionNames = device_extensions_.data();
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.pEnabledFeatures = &device_features;
  device_info.pNext = &device_features12;

  VkDevice device;
  VkResult res = vkCreateDevice(phys_device, &device_info, &allocator_, &device);
  ASSERTF(res == VK_SUCCESS, "vkCreateDevice error %d\n", res);

  return device;
}

auto gdm::vk::Device::CreateQueue(int queue_family_idx) -> VkQueue
{
  VkQueue queue;
  vkGetDeviceQueue(device_, queue_family_idx, 0, &queue);
  return queue;
}

// --helpers

auto gdm::vk::helpers::ValidateDeviceExtensionsProps(const Device& device, const std::vector<const char*>& extensions_info) -> std::vector<size_t>
{
  uint32_t extensions_count = 0;
  vkEnumerateDeviceExtensionProperties(device, NULL, &extensions_count, NULL);
  ASSERTF(extensions_count != 0, "Failed to find any device extensions");

  std::vector<VkExtensionProperties> extensions_available(extensions_count);
  VkResult res = vkEnumerateDeviceExtensionProperties(device, NULL, &extensions_count, extensions_available.data());
  ASSERTF(res == VK_SUCCESS, "vkEnumerateInstanceExtensionProperties error %d", res);

  std::vector<size_t> unsupported;

  for (size_t i = 0; i < extensions_info.size(); ++i)
  {
    bool found = false;
    for (size_t k = 0; k < extensions_available.size() && !found; ++k)
      found = strcmp(extensions_info[i], extensions_available[k].extensionName) == 0;
    if (!found)
      unsupported.push_back(i);
  }
  return unsupported;
}
