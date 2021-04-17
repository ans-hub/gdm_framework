// *************************************************************
// File:    vk_renderer.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_RENDERER_H
#define GM_VK_RENDERER_H

#include <map>
#include <vector>
#include <functional>

#include "render/defines.h"

#include "vk_pointers.h"
#include "vk_defines.h"
#include "vk_device.h"
#include "vk_physical_device.h"
#include "vk_command_list.h"
#include "vk_image.h"
#include "vk_image_view.h"
#include "vk_fence.h"

#include "system/hash_utils.h"
#include "threads/fence.h"

namespace gdm::vk {

struct Renderer
{
  enum class Event : uint
  {
    SwapchainRecreated = 0,
    Max
  }; // enum class Event

  Renderer(HWND window_handle, gfx::DeviceProps flags = 0);
  ~Renderer();

  auto GetSurfaceWidth() -> uint;
  auto GetSurfaceHeight() -> uint;
  auto GetSurfaceFormat() const -> gfx::EFormatType;
  auto GetSurfaceWindow() const -> HWND { return surface_create_info_.hwnd; }
  auto GetPresentImageFormat() const -> VkFormat;
  auto GetDevice() -> Device&;
  auto GetInstance() const -> VkInstance { return instance_; }
  auto GetDescriptorPool() -> VkDescriptorPool;
  auto GetBackBufferViews() -> std::vector<ImageView*>;
  auto GetBackBufferImages() -> std::vector<VkImage>;
  auto CreateFrameCommandList(uint frame_num, gfx::CommandListFlags flags) -> CommandList;
  auto CreateCommandList(Hash name, gfx::CommandListFlags flags) -> CommandList;
  auto AcquireNextFrame(VkSemaphore sem_sig, VkFence fence) -> uint;
  void SubmitCommandLists(const std::vector<VkCommandBuffer>& command_lists, const std::vector<VkSemaphore>& sem_wait, const std::vector<VkSemaphore>& sem_sig, VkFence fence);
  void SubmitPresentation(uint frame_num, const std::vector<VkSemaphore>& sem_wait);
  void WaitForGpuIdle();
  void FlushGpuEvents(std::vector<Event>& out) { out.insert(out.end(), gpu_events_.begin(), gpu_events_.end()); gpu_events_.clear(); } 

  void BeginDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color);
  void InsertDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color);
  void EndDebugLabel(VkCommandBuffer cmd);
  
  auto GetBackBuffersCount() const -> uint { return v_num_images_; };

private:
  auto CreateInstance() -> VkInstance;
  auto CreateSurfaceInfo(HWND window_handle) -> VkWin32SurfaceCreateInfoKHR;
  auto CreateSurface(VkInstance instance) -> VkSurfaceKHR;
  auto CreateDevice(PhysicalDeviceId phys_device) -> Device*;
  auto CreateSwapChain(const PhysicalDevice& phys_device, unsigned num_images) -> VkSwapchainKHR;
  auto CreatePresentImages() -> std::vector<VkImage>;
  auto CreatePresentImagesView() -> std::vector<ImageView*>;
  auto CreateCommandPool(bool reset_on_begin, uint queue_index, VkCommandPoolCreateFlagBits flags) -> VkCommandPool;
  auto CreateCommandBuffers(VkCommandPool pool, uint count) -> std::vector<VkCommandBuffer>;
  auto CreateDescriptorPool(int max_sets, const std::vector<std::pair<gfx::EResourceType, uint>>& pools_description) -> VkDescriptorPool;
  auto CreateDebugCallbackLegacy() -> VkDebugReportCallbackEXT;
  auto CreateDebugCallbackModern() -> VkDebugUtilsMessengerEXT;
  void InitializePresentImages();
  auto FillInstanceLayersInfo() -> std::vector<const char*>;
  auto FillInstanceExtensionsInfo() -> std::vector<const char*>;
  void ValidateSetup();
  void CleanupSwapChain();
  void CleanupCommandPools();
  void RecreateSwapChain();
  void UpdatePhysicalDevicesInfo();

private:
  unsigned v_num_images_;
  VkAllocationCallbacks allocator_;
  std::vector<const char*> instance_extensions_;
  std::vector<const char*> instance_layers_;
  VkInstance instance_;
  VkDebugUtilsMessengerEXT debug_callback_modern_;
  VkDebugReportCallbackEXT debug_callback_legacy_;
  VkWin32SurfaceCreateInfoKHR surface_create_info_;
  VkSurfaceKHR surface_;
  VkQueueFlags queue_flags_;
  std::vector<api::PhysicalDevice> phys_devices_db_;
  api::PhysicalDeviceId phys_device_;  
  api::Device* device_;
  api::Fence* submit_fence_;
  VkSwapchainKHR swapchain_;
  std::vector<VkImage> present_images_;
  std::vector<api::ImageView*> present_images_views_;
  std::vector<Event> gpu_events_; 

private:
  struct Tls
  {
    VkCommandPool setup_command_pool_;
    VkCommandPool frame_command_pool_;
    std::map<Hash, VkCommandBuffer> setup_command_buffers_;
    std::vector<VkCommandBuffer> frame_command_buffers_;
    VkDescriptorPool descriptor_pool_;
  };

  std::vector<Tls> tls_;

}; // struct Renderer

namespace helpers {

  auto EnumerateInstanceExtensionsProps() -> std::vector<VkExtensionProperties>;
  auto FindUnsupportedLayers(const std::vector<const char*>& layers_info) -> std::vector<size_t>;
  auto FindUnsupportedInstanceExtensions(const std::vector<const char*>& extensions_info) -> std::vector<size_t>;
  void RemoveUnsupportedLayers(std::vector<const char*>& layers_info);

} // namespace helpers

} // namespace gdm::vk

#endif // GM_VK_RENDERER_H
