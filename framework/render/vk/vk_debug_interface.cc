// *************************************************************
// File:    vk_debug_interface.cc
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_debug_interface.h"

//--private

void gdm::vk::debug::BeginDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color)
{
  VkDebugUtilsLabelEXT markerInfo = {};
  markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  memcpy(markerInfo.color, &color.data[0], sizeof(Vec4f));
  markerInfo.pLabelName = name;
  vkCmdBeginDebugUtilsLabel(cmd, &markerInfo);
}

void gdm::vk::debug::InsertDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color)
{
  VkDebugUtilsLabelEXT markerInfo = {};
  markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  memcpy(markerInfo.color, &color.data[0], sizeof(Vec4f));
  markerInfo.pLabelName = name;
  vkCmdInsertDebugUtilsLabel(cmd, &markerInfo);
}

void gdm::vk::debug::EndDebugLabel(VkCommandBuffer cmd)
{
  vkCmdEndDebugUtilsLabel(cmd);
}
