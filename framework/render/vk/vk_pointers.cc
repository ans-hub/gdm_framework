// *************************************************************
// File:    vk_pointers.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_pointers.h"

extern PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTag = nullptr;
extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel = nullptr;
extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel = nullptr;
extern PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel = nullptr;

void gdm::vk::GatherAdditionalVulkanFunctionPtrs(VkDevice device)
{
  vkSetDebugUtilsObjectTag = PFN_vkSetDebugUtilsObjectTagEXT(vkGetDeviceProcAddr(device, "vkSetDebugUtilsObjectTagEXT"));
  vkCmdBeginDebugUtilsLabel = PFN_vkCmdBeginDebugUtilsLabelEXT(vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
  vkCmdEndDebugUtilsLabel = PFN_vkCmdEndDebugUtilsLabelEXT(vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
  vkCmdInsertDebugUtilsLabel = PFN_vkCmdInsertDebugUtilsLabelEXT(vkGetDeviceProcAddr(device, "vkCmdInsertDebugUtilsLabelEXT"));
}