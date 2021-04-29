
// *************************************************************
// File:    vk_debug_interface.inl
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_debug_interface.h"

#include "system/assert_utils.h"

template<class T>
void gdm::vk::debug::SetDebugName(VkDevice device, T vk_handle, const char* name)
{
  if constexpr (gfx::v_DebugBuild)
  {
    VkObjectType object_type;

    if constexpr(std::is_same_v<VkBuffer, T>)
      object_type = VK_OBJECT_TYPE_BUFFER;
    else if constexpr (std::is_same_v<VkImage, T>)
      object_type = VK_OBJECT_TYPE_IMAGE;
    else if constexpr (std::is_same_v<VkImageView, T>)
      object_type = VK_OBJECT_TYPE_IMAGE_VIEW;
    else if constexpr (std::is_same_v<VkCommandBuffer, T>)
      object_type = VK_OBJECT_TYPE_COMMAND_BUFFER;
    else
    {
      ENSURE("Not implemented");
    return;
    }
		
    VkDebugUtilsObjectNameInfoEXT info = {};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		info.objectType = object_type;
		info.objectHandle = (uint64_t)vk_handle;
		info.pObjectName = name;

    VkResult res = vkSetDebugUtilsObjectName(device, &info);
    ASSERTF(res == VK_SUCCESS, "vkSetDebugUtilsObjectName %d", res);
  }
}
