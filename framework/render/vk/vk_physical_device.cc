// *************************************************************
// File:    vk_physical_device.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_physical_device.h"

#include <system/assert_utils.h>

// --helpers

auto gdm::vk::helpers::EnumeratePhysicalDevices(VkInstance instance, VkSurfaceKHR surface) -> std::vector<PhysicalDevice>
{
  uint num = 0;
  VkResult res = vkEnumeratePhysicalDevices(instance, &num, nullptr);
  ASSERTF(res == VK_SUCCESS, "vkEnumeratePhysicalDevices error %d\n", res);
  
  std::vector<PhysicalDevice> devices_db(num);
  res = vkEnumeratePhysicalDevices(instance, &num, &devices_db[0].device_);
  ASSERTF(res == VK_SUCCESS, "vkEnumeratePhysicalDevices error %d\n", res);

  for (uint i = 0 ; i < num ; i++)
  {
    VkPhysicalDevice device = devices_db[i].device_;
    vkGetPhysicalDeviceProperties(device, &devices_db[i].device_props_);
    vkGetPhysicalDeviceProperties2(device, &devices_db[i].device_props2_);
    vkGetPhysicalDeviceMemoryProperties(device, &devices_db[i].device_mem_props_);

    uint qnum = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qnum, nullptr);
    
    devices_db[i].queue_family_props_.resize(qnum);
    devices_db[i].queue_supports_present_.resize(qnum);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &qnum, &(devices_db[i].queue_family_props_[0]));
    
    for (uint q = 0 ; q < qnum ; q++)
    {
      res = vkGetPhysicalDeviceSurfaceSupportKHR(device, q, surface, &(devices_db[i].queue_supports_present_[q]));
      ASSERTF(res == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceSupportKHR error %d\n", res);
    }

    uint fnum = 0;
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fnum, nullptr);
    ASSERTF(res == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error %d\n", res);
    
    devices_db[i].surface_formats_.resize(fnum);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &fnum, &(devices_db[i].surface_formats_[0]));
    ASSERTF(res == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceFormatsKHR error %d\n", res);

    bool only_undefined_format = true;
    only_undefined_format &= (devices_db[i].surface_formats_.size() == 1);
    only_undefined_format &= (devices_db[i].surface_formats_[0].format == VK_FORMAT_UNDEFINED);

    if (only_undefined_format)
      devices_db[i].surface_formats_[0].format = VK_FORMAT_B8G8R8_UNORM;

    res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &(devices_db[i].surface_caps_));
    ASSERTF(res == VK_SUCCESS, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR error %d\n", res);

    uint mode_cnt = 0;
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_cnt, nullptr);
    ASSERTF(res == VK_SUCCESS, "vkGetPhysicalDeviceSurfacePresentModesKHR error %d\n", res);

    devices_db[i].presentation_modes_.resize(mode_cnt);
    res = vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &mode_cnt, devices_db[i].presentation_modes_.data());
    ASSERTF(res == VK_SUCCESS, "vkGetPhysicalDeviceSurfacePresentModesKHR error %d\n", res);
  }

  return devices_db;
}

// --helpers

auto gdm::vk::helpers::FindPhysicalDeviceId(const std::vector<PhysicalDevice>& db, VkQueueFlags req_flags, unsigned num_images) -> PhysicalDeviceId
{
  DeviceId device_id = -1;
  QueueId queue_id = -1;

  for (size_t i = 0; i < db.size() && device_id == -1; ++i)
  {
    const PhysicalDevice& device = db[i]; 
    
    for (size_t j = 0; j < device.queue_family_props_.size(); ++j)
    {
      bool flags_ok = device.queue_family_props_[j].queueFlags & req_flags;
      bool present_ok = device.queue_supports_present_[j];
      bool images_ok = true;
      images_ok &= device.surface_caps_.minImageCount <= num_images;
      images_ok &= device.surface_caps_.maxImageCount >= num_images;

      if (flags_ok && present_ok && images_ok)
      {
        device_id = static_cast<int>(i);
        queue_id = static_cast<int>(j);
        break;
      }
    }
  }
  
  ASSERTF(queue_id != -1, "Queue index == -1");
  ASSERTF(device_id != -1, "Device index == -1");
  
  return {device_id, queue_id, db[device_id]};
}

auto gdm::vk::helpers::FindMemoryIndex(const VkPhysicalDeviceMemoryProperties& mem_props, uint32_t mem_type_bits_requirment, VkMemoryPropertyFlags required_props) -> uint32_t
{
  const uint32_t mem_count = mem_props.memoryTypeCount;

  for (uint32_t mem_index = 0; mem_index < mem_count; ++mem_index)
  {
    const uint32_t mem_type_bits = (1 << mem_index);
    const bool is_required_mem_type = mem_type_bits_requirment & mem_type_bits;

    const VkMemoryPropertyFlags properties = mem_props.memoryTypes[mem_index].propertyFlags;
    const bool has_required_props = (properties & required_props) == required_props;

    if (is_required_mem_type && has_required_props)
      return static_cast<int32_t>(mem_index);
  }
  return -1;
}
