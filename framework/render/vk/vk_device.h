// *************************************************************
// File:    vk_device.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_DEVICE_H
#define GM_VK_DEVICE_H

#include <vector>

#include "render/defines.h"
#include "render/vk/vk_defines.h"
#include "render/vk/vk_physical_device.h"

namespace gdm::vk {

struct Device
{
  Device(const PhysicalDeviceId& phys_device);
  ~Device();

  Device(const Device& other) = delete;
  Device& operator=(const Device& other) = delete;

  auto GetPtr() const -> VkDevice { return device_; }
  auto GetPhysicalDevice() const -> const PhysicalDeviceId& { return phys_device_; }
  auto GetQueue(gfx::QueueType type) -> VkQueue;

  operator VkDevice() { return device_; }
  operator VkPhysicalDevice() const { return phys_device_.info_.device_; }

private:
  auto CreateLogicalDevice(VkPhysicalDevice phys_device, int queue_family_idx) -> VkDevice;
  auto CreateQueue(int queue_family_idx) -> VkQueue;
  auto FillDeviceExtensionsInfo() const -> std::vector<const char*>;

private:
  VkAllocationCallbacks allocator_;
  PhysicalDeviceId phys_device_;
  std::vector<const char*> device_extensions_;
  VkDevice device_;
  VkQueue graphics_queue_;
  VkQueue presentation_queue_;
 
}; // struct Device

namespace helpers {

  auto ValidateDeviceExtensionsProps(const Device& device, const std::vector<const char*>& extensions_info) -> std::vector<size_t>;

} // namespace helpers

} // namespace gdm::vk

#endif // GM_VK_DEVICE_H
