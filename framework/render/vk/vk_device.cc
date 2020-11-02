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
}

// --private

auto gdm::vk::Device::FillDeviceExtensionsInfo() const -> std::vector<const char*>
{
  std::vector<const char*> device_extensions;
  device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
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
    
  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions_.size());
  device_info.ppEnabledExtensionNames = device_extensions_.data();
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;

  VkPhysicalDeviceFeatures device_features = {};
  device_features.shaderClipDistance = VK_TRUE;
  VkPhysicalDeviceVulkan12Features device_features12 = {};
  device_features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
  device_features12.timelineSemaphore = true;
  device_features12.descriptorBindingSampledImageUpdateAfterBind = true;

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

auto gdm::vk::helpers::EnumerateDeviceExtensionsProps(const Device& device) -> std::vector<VkExtensionProperties>
{
  uint num = 0;
  VkResult res = vkEnumerateDeviceExtensionProperties(device, nullptr, &num, nullptr);
  ASSERTF(res == VK_SUCCESS, "vkEnumerateDeviceExtensionProperties error %d\n", res);
  
  std::vector<VkExtensionProperties> props(num);
  res = vkEnumerateDeviceExtensionProperties(device, nullptr, &num, &props[0]);
  ASSERTF(res == VK_SUCCESS, "vkEnumerateDeviceExtensionProperties error %d\n", res);

  return props;
}
