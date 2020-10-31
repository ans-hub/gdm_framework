// *************************************************************
// File:    vk_physical_device.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_PHYS_DEVICE_H
#define GM_VK_PHYS_DEVICE_H

#include <vector>

#include "vk_defines.h"

namespace gdm::vk {

struct PhysicalDevice
{
  VkPhysicalDevice device_;
  VkPhysicalDeviceProperties device_props_;
  VkPhysicalDeviceMemoryProperties device_mem_props_;
  std::vector<VkQueueFamilyProperties> queue_family_props_;
  std::vector<VkBool32> queue_supports_present_;
  std::vector<VkSurfaceFormatKHR> surface_formats_;
  std::vector<VkPresentModeKHR> presentation_modes_;
  VkSurfaceCapabilitiesKHR surface_caps_;

}; // struct PhysicalDevice

using DeviceId = int;
using QueueId = int;

struct PhysicalDeviceId
{
  DeviceId device_id;
  QueueId queue_id;
  const PhysicalDevice& info_;

}; // struct PhysicalDeviceId

namespace helpers {

  auto EnumeratePhysicalDevices(VkInstance instance, VkSurfaceKHR surface) -> std::vector<PhysicalDevice>;
  auto FindPhysicalDeviceId(const std::vector<PhysicalDevice>& db, VkQueueFlags req_flags, unsigned num_images) -> PhysicalDeviceId;
  auto FindMemoryIndex(const VkPhysicalDeviceMemoryProperties& mem_props, uint32_t mem_type_bits_requirment, VkMemoryPropertyFlags required_props) -> uint32_t;

} // namespace helpers

} // namespace gdm::vk

#endif // GM_VK_PHYS_DEVICE_H
