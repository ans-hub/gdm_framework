// *************************************************************
// File:    vk_pointers.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_POINTERS_H
#define GM_VK_POINTERS_H

#include <render/defines.h>

extern PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTag;
extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel;
extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel;
extern PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel;

namespace gdm::vk
{
  void GatherAdditionalVulkanFunctionPtrs(VkDevice device);
}

#endif // GM_VK_POINTERS_H
