// *************************************************************
// File:    vk_renderer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_RENDERER_H
#define GM_VK_RENDERER_H

#include <map>

#include "render/defines.h"

#include "vk_defines.h"
#include "vk_device.h"
#include "vk_physical_device.h"
#include "vk_command_list.h"
#include "vk_image.h"
#include "vk_fence.h"

#include "system/hash_utils.h"

namespace gdm::vk {

struct Renderer
{
  Renderer(HWND window_handle, gfx::DeviceProps flags = 0);
  ~Renderer();

  auto GetSurfaceWidth() -> uint;
  auto GetSurfaceHeight() -> uint;
  auto GetSurfaceFormat() const -> gfx::EFormatType;
  auto GetDevice() -> Device&;
  auto GetDescriptorPool() -> VkDescriptorPool;
  auto GetBackBufferViews() -> std::vector<VkImageView>;
  auto GetBackBufferImages() -> std::vector<VkImage>;
  auto GetBackBuffersCount() const -> uint { return v_num_images_; };
  auto CreateFrameCommandList(uint frame_num, gfx::CommandListFlags flags) -> CommandList;
  auto CreateSetupCommandList(Hash name, gfx::CommandListFlags flags) -> CommandList;
  auto AcquireNextFrame(VkSemaphore sem_sig, VkFence fence) -> uint;
  void SubmitCommandLists(const std::vector<VkCommandBuffer>& command_lists, const std::vector<VkSemaphore>& sem_wait, const std::vector<VkSemaphore>& sem_sig, VkFence fence);
  void SubmitPresentation(uint frame_num, const std::vector<VkSemaphore>& sem_wait);

private:
  auto CreateInstance() -> VkInstance;
  auto CreateSurface(HWND window_handle, VkInstance instance) -> VkSurfaceKHR;
  auto CreateDevice(PhysicalDeviceId phys_device) -> Device*;
  auto CreateSwapChain(const PhysicalDevice& phys_device, unsigned num_images) -> VkSwapchainKHR;
  auto CreatePresentImages() -> std::vector<VkImage>;
  auto CreatePresentImagesView() -> std::vector<VkImageView>;
  auto CreateCommandPool(bool reset_on_begin, uint queue_index, VkCommandPoolCreateFlagBits flags) -> VkCommandPool;
  auto CreateCommandBuffers(VkCommandPool pool, uint count) -> std::vector<VkCommandBuffer>;
  auto CreateDescriptorPool(int max_sets, const std::vector<std::pair<gfx::EResourceType, uint>>& pools_description) -> VkDescriptorPool;
  auto CreateDebugCallback() -> VkDebugReportCallbackEXT;
  void InitializePresentImages();
  auto FillInstanceLayersInfo() -> std::vector<const char*>;
  auto FillInstanceExtensionsInfo() -> std::vector<const char*>;
  void Cleanup();
  
private:
  unsigned v_num_images_;
  VkAllocationCallbacks allocator_;
  std::vector<const char*> instance_extensions_;
  std::vector<const char*> instance_layers_;
  VkInstance instance_;
  VkDebugReportCallbackEXT debug_callback_; 
  VkSurfaceKHR surface_;
  VkQueueFlags queue_flags_;
  std::vector<PhysicalDevice> phys_devices_db_;
  PhysicalDeviceId phys_device_;  
  Device* device_;
  Fence submit_fence_;
  VkSwapchainKHR swapchain_;
  std::vector<VkImage> present_images_;
  std::vector<VkImageView> present_images_views_;

private:
  static thread_local VkCommandPool setup_command_pool_;
  static thread_local VkCommandPool frame_command_pool_;
  static thread_local std::vector<VkCommandBuffer> frame_command_buffers_;
  static thread_local std::map<Hash, VkCommandBuffer> setup_command_buffers_;
  static thread_local VkDescriptorPool descriptor_pool_;

}; // struct Renderer

namespace helpers {

  auto EnumerateInstanceExtensionsProps() -> std::vector<VkExtensionProperties>;
  bool ValidateInstanceLayersInfo(const std::vector<const char*>& layers_info);
  bool ValidateInstanceExtensionsInfo(const std::vector<const char*>& extensions_info);
  
} // namespace helpers

} // namespace gdm::vk

#endif // GM_VK_RENDERER_H
