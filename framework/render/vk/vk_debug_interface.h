// *************************************************************
// File:    vk_debug_interface.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_DEBUG_INTERFACE_H
#define GM_VK_DEBUG_INTERFACE_H

#include "math/vector4.h"

#include "render/defines.h"

#include "vk/vk_pointers.h"

namespace gdm::vk::debug {

  void BeginDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color);
  void InsertDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color);
  void EndDebugLabel(VkCommandBuffer cmd);

  template<class T>
  void SetDebugName(VkDevice device, T vk_handle, const char* name);

} // namespace gdm::vk::debug

#include "vk_debug_interface.inl"

#endif // GM_VK_DEBUG_INTERFACE_H
