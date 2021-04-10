// *************************************************************
// File:    vk_renderer.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "vk_renderer.h"

#include <memory/defines.h>
#include <system/assert_utils.h>
#include <render/vk/vk_defines.h>
#include <render/vk/vk_command_list.h>
#include <render/vk/vk_host_allocator.h>
#include <render/vk/vk_fence.h>
#include <render/vk/vk_semaphore.h>
#include <render/vk/vk_barrier.h>
#include <render/vk/vk_image.h>
#include <render/vk/vk_image_view.h>
#include <render/vk/vk_render_pass.h>
#include <render/vk/vk_framebuffer.h>

// --private

namespace gdm::_private {

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportLegacyCB(
    VkDebugReportFlagsEXT       flags,
    VkDebugReportObjectTypeEXT  object_type,
    uint64_t                    object,
    size_t                      location,
    int32_t                     message_code,
    const char*                 layer_prefix,
    const char*                 message,
    void*                       user_data)
  {
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    return VK_FALSE;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportModernCB(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
  {
    OutputDebugStringA(callback_data->pMessage);
    OutputDebugStringA("\n");
    return VK_FALSE;
  }

} // namespace gdm::_private

thread_local VkCommandPool gdm::vk::Renderer::setup_command_pool_ = VK_NULL_HANDLE;
thread_local VkCommandPool gdm::vk::Renderer::frame_command_pool_ = VK_NULL_HANDLE;
thread_local std::vector<VkCommandBuffer> gdm::vk::Renderer::frame_command_buffers_ = {};
thread_local std::map<gdm::Hash, VkCommandBuffer> gdm::vk::Renderer::setup_command_buffers_ = {};
thread_local VkDescriptorPool gdm::vk::Renderer::descriptor_pool_ = VK_NULL_HANDLE;

// --public

gdm::vk::Renderer::Renderer(HWND window_handle, gfx::DeviceProps flags /*=0*/)
  : v_num_images_{gfx::v_num_images}
  , allocator_{ *HostAllocator::GetPtr() }
  , instance_extensions_{ FillInstanceExtensionsInfo() }
  , instance_layers_{ FillInstanceLayersInfo() }
  , instance_{ CreateInstance() }
  , debug_callback_modern_{ CreateDebugCallbackModern() }
  , debug_callback_legacy_{ debug_callback_modern_ ? nullptr : CreateDebugCallbackLegacy() }
  , surface_{ CreateSurface(window_handle, instance_) }
  , queue_flags_{ VK_QUEUE_GRAPHICS_BIT }
  , phys_devices_db_{ helpers::EnumeratePhysicalDevices(instance_, surface_) }
  , phys_device_{ helpers::FindPhysicalDeviceId(phys_devices_db_, queue_flags_, v_num_images_) }
  , device_{ CreateDevice(phys_device_) }
  , submit_fence_(*device_)
  , swapchain_{ CreateSwapChain(phys_device_.info_, v_num_images_) }
  , present_images_{ CreatePresentImages() }
  , present_images_views_{ CreatePresentImagesView() }
{ 
  InitializePresentImages();
  GatherAdditionalVulkanFunctionPtrs(*device_);
  ValidateSetup();
}

gdm::vk::Renderer::~Renderer()
{
  Cleanup();
}

auto gdm::vk::Renderer::GetSurfaceWidth() -> uint
{
  return phys_device_.info_.surface_caps_.currentExtent.width;
}

auto gdm::vk::Renderer::GetSurfaceHeight() -> uint
{
  return phys_device_.info_.surface_caps_.currentExtent.height;
}

auto gdm::vk::Renderer::GetSurfaceFormat() const -> gfx::EFormatType
{
  const VkSurfaceFormatKHR& surface_formats = phys_device_.info_.surface_formats_[0];
  return static_cast<gfx::EFormatType>(surface_formats.format);
}

auto gdm::vk::Renderer::GetDevice() -> Device&
{
  return *device_;
}

auto gdm::vk::Renderer::GetBackBufferViews() -> std::vector<ImageView*>
{
  return present_images_views_;
}

auto gdm::vk::Renderer::GetBackBufferImages() -> std::vector<VkImage>
{
  return present_images_;
}

auto gdm::vk::Renderer::CreateFrameCommandList(uint frame_num, gfx::CommandListFlags flags) -> CommandList
{
  if (frame_command_pool_ == VK_NULL_HANDLE)
  {
    frame_command_pool_ = CreateCommandPool(true, phys_device_.queue_id, static_cast<VkCommandPoolCreateFlagBits>(0));
    frame_command_buffers_ = CreateCommandBuffers(frame_command_pool_, v_num_images_);
  }
  return CommandList(*device_, frame_command_buffers_[frame_num], flags);
}

auto gdm::vk::Renderer::CreateCommandList(Hash name, gfx::CommandListFlags flags) -> CommandList
{
  if (setup_command_pool_ == VK_NULL_HANDLE)
    setup_command_pool_ = CreateCommandPool(true, phys_device_.queue_id, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
  if (setup_command_buffers_.find(name) == setup_command_buffers_.end())
    setup_command_buffers_[name] = CreateCommandBuffers(setup_command_pool_, 1).back();

  return CommandList(*device_, setup_command_buffers_[name], flags);
}

auto gdm::vk::Renderer::GetDescriptorPool() -> VkDescriptorPool // todo: customize
{
  if (descriptor_pool_ == VK_NULL_HANDLE)
  {
    uint max_sampled_images = phys_device_.info_.device_props_.limits.maxPerStageDescriptorSampledImages; 
    uint max_dynamic_uniforms = phys_device_.info_.device_props_.limits.maxDescriptorSetUniformBuffers; 
    
    max_sampled_images = min(max_sampled_images, gfx::v_max_descriptor_set_alloc);
    max_dynamic_uniforms = min(max_dynamic_uniforms, gfx::v_max_descriptor_set_alloc);

    descriptor_pool_ = CreateDescriptorPool(16,
      {
        { gfx::EResourceType::SAMPLER, 6 },
        { gfx::EResourceType::SAMPLED_IMAGE, max_sampled_images },
        { gfx::EResourceType::UNIFORM_DYNAMIC, max_dynamic_uniforms }
      }
    );
  }
  return descriptor_pool_;
}

auto gdm::vk::Renderer::AcquireNextFrame(VkSemaphore sem_sig, VkFence fence) -> uint
{
  uint curr_image = 0;
  VkResult res = vkAcquireNextImageKHR(*device_, swapchain_, UINT64_MAX, sem_sig, NULL, &curr_image);
  ASSERTF(res == VK_SUCCESS, "vkAcquireNextImageKHR %d\n", res);
  return curr_image;
}

void gdm::vk::Renderer::SubmitCommandLists(const std::vector<VkCommandBuffer>& command_lists, const std::vector<VkSemaphore>& sem_wait, const std::vector<VkSemaphore>& sem_sig, VkFence fence)
{
  VkPipelineStageFlags wait_stage_mask = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
  VkSubmitInfo submit_info = {};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = static_cast<uint32_t>(sem_wait.size());
  submit_info.pWaitSemaphores = sem_wait.data();
  submit_info.pWaitDstStageMask = &wait_stage_mask;
  submit_info.signalSemaphoreCount = static_cast<uint32_t>(sem_sig.size());
  submit_info.pSignalSemaphores = sem_sig.data();
  submit_info.commandBufferCount = static_cast<uint32_t>(command_lists.size());
  submit_info.pCommandBuffers = command_lists.data();

  VkResult res = vkQueueSubmit(device_->GetQueue(gfx::GRAPHICS), 1, &submit_info, fence);
  ASSERTF(res == VK_SUCCESS, "vkQueueSubmit %d\n", res);
}

void gdm::vk::Renderer::SubmitPresentation(uint frame_num, const std::vector<VkSemaphore>& sem_wait)
{
  VkPresentInfoKHR present_info = {};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = static_cast<uint32_t>(sem_wait.size());
  present_info.pWaitSemaphores = sem_wait.data();
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &swapchain_;
  present_info.pImageIndices = &frame_num;
  
  VkResult res = vkQueuePresentKHR(device_->GetQueue(gfx::PRESENTATION), &present_info);   // queue image for presentation  
  ASSERTF(res == VK_SUCCESS, "vkQueuePresentKHR %d\n", res);
}

void gdm::vk::Renderer::WaitForGpuIdle()
{
  VkResult res = vkDeviceWaitIdle(*device_);
  ASSERTF(res == VK_SUCCESS, "vkDeviceWaitIdle %d\n", res);
}

void gdm::vk::Renderer::BeginDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color)
{
  VkDebugUtilsLabelEXT markerInfo = {};
  markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  memcpy(markerInfo.color, &color.data[0], sizeof(Vec4f));
  markerInfo.pLabelName = name;
  vkCmdBeginDebugUtilsLabel(cmd, &markerInfo);
}

void gdm::vk::Renderer::InsertDebugLabel(VkCommandBuffer cmd, const char* name, const Vec4f& color)
{
  VkDebugUtilsLabelEXT markerInfo = {};
  markerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
  memcpy(markerInfo.color, &color.data[0], sizeof(Vec4f));
  markerInfo.pLabelName = name;
  vkCmdInsertDebugUtilsLabel(cmd, &markerInfo);
}

void gdm::vk::Renderer::EndDebugLabel(VkCommandBuffer cmd)
{
  vkCmdEndDebugUtilsLabel(cmd);
}

// --private

void gdm::vk::Renderer::ValidateSetup()
{
  if constexpr (gfx::v_DebugBuild)
  {
    const bool valid_debug_cb = debug_callback_legacy_ || debug_callback_modern_;
    ENSUREF(valid_debug_cb, "No one valid debug callback for validation layer was provided");
  }
}

auto gdm::vk::Renderer::CreateInstance() -> VkInstance
{
  VkApplicationInfo app_info = {};       
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "gdm::VkRenderer";
  app_info.engineVersion = 1;
  app_info.apiVersion = VK_API_VERSION_1_2;
    
  VkInstanceCreateInfo instance_info = {};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &app_info;
  instance_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions_.size());
  instance_info.ppEnabledExtensionNames = instance_extensions_.data();

  if constexpr (gfx::v_DebugBuild)
  {
    instance_info.enabledLayerCount = static_cast<uint32_t>(instance_layers_.size());
    instance_info.ppEnabledLayerNames = instance_layers_.data();
  }

  VkInstance instance = nullptr;
  VkResult res = vkCreateInstance(&instance_info, &allocator_, &instance);
  ASSERTF(res == VK_SUCCESS, "vkCreateInstance %d\n", res);

  return instance;
}

auto gdm::vk::Renderer::CreateSurface(HWND window_handle, VkInstance instance) -> VkSurfaceKHR
{
  surface_create_info_ = {};

  surface_create_info_.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  surface_create_info_.hinstance = GetModuleHandle(nullptr);
  surface_create_info_.hwnd = window_handle;
  
  VkSurfaceKHR surface;
  
  VkResult res = vkCreateWin32SurfaceKHR(instance, &surface_create_info_, &allocator_, &surface);
  ASSERTF(res == VK_SUCCESS, "vkCreateXcbSurfaceKHR error %d\n", res);
  
  return surface;
}

auto gdm::vk::Renderer::CreateDevice(PhysicalDeviceId phys_device) -> Device*
{
  return GMNew Device(phys_device);
}

auto gdm::vk::Renderer::CreateSwapChain(const PhysicalDevice& phys_device, unsigned num_images) -> VkSwapchainKHR
{
  const VkSurfaceCapabilitiesKHR& surface_caps = phys_device.surface_caps_;
  const VkSurfaceFormatKHR& surface_formats = phys_device.surface_formats_[0];

  ASSERTF(surface_caps.currentExtent.width != -1, "Surface capabilities == -1");
  ASSERTF(surface_caps.minImageCount <= static_cast<unsigned>(num_images), "Incompatible min images count");
  ASSERTF(surface_caps.maxImageCount >= static_cast<unsigned>(num_images), "Incompatible max images count");

  VkSwapchainCreateInfoKHR swapchain_create_info = {};
    
  swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_create_info.surface = surface_;
  swapchain_create_info.minImageCount = num_images;
  swapchain_create_info.imageFormat = surface_formats.format;
  swapchain_create_info.imageColorSpace = surface_formats.colorSpace;
  swapchain_create_info.imageExtent = surface_caps.currentExtent;
  swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  
  if (surface_caps.currentTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    swapchain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

  swapchain_create_info.imageArrayLayers = 1;
  swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
  for (auto mode : phys_device.presentation_modes_)
    if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
      present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
  swapchain_create_info.presentMode = present_mode;

  swapchain_create_info.clipped = true;
  swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;  

  VkSwapchainKHR swapchain {};
  VkResult res = vkCreateSwapchainKHR(*device_, &swapchain_create_info, &allocator_, &swapchain);
  ASSERTF(res == VK_SUCCESS, "vkCreateSwapchainKHR error %d\n", res);

  return swapchain;
}

auto gdm::vk::Renderer::CreatePresentImages() -> std::vector<VkImage>
{
  VkResult res = vkGetSwapchainImagesKHR(*device_, swapchain_, &v_num_images_, nullptr);  
  ASSERTF(res == VK_SUCCESS, "vkGetSwapchainImagesKHR error %d\n", res);

  std::vector<VkImage> images(v_num_images_);

  res = vkGetSwapchainImagesKHR(*device_, swapchain_, &v_num_images_, &(images[0]));
  ASSERTF(res == VK_SUCCESS, "vkGetSwapchainImagesKHR error %d\n", res);

  return images;
}

// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#vkAcquireNextImageKHR

void gdm::vk::Renderer::InitializePresentImages()
{
  int transitions_total = 0;
  std::vector<bool> transition_done(present_images_.size(), false);

  while (transitions_total != transition_done.size())
  {
    Semaphore present_complete_sem(*device_);
    uint frame_num = AcquireNextFrame(present_complete_sem, vk::Fence::null);
    
    if (!transition_done[frame_num])
    {
      ImageBarrier barrier;

      barrier.GetProps()
        .AddImage(present_images_[frame_num])
        .AddOldLayout(gfx::EImageLayout::UNDEFINED)
        .AddNewLayout(gfx::EImageLayout::PRESENT_SRC)
        .Finalize();

      CommandList list = CreateCommandList(GDM_HASH("SetupRenderer"), gfx::ECommandListFlags::ONCE);
      list.PushBarrier(barrier);
      list.Finalize();

      SubmitCommandLists(vk::CommandLists{list}, vk::Semaphores{present_complete_sem}, vk::Semaphores::empty, submit_fence_);
      submit_fence_.WaitSignalFromGpu();
      submit_fence_.Reset();
      
      transition_done[frame_num] = true;
      ++transitions_total;
    }
    SubmitPresentation(frame_num, vk::Semaphores::empty);
  }
}

auto gdm::vk::Renderer::CreatePresentImagesView() -> std::vector<ImageView*>
{
  std::vector<ImageView*> views(present_images_.size());

  for (size_t i = 0; i < present_images_.size(); ++i)
  {
    views[i] = GMNew ImageView(*device_);
    views[i]->GetProps()
      .AddImage(present_images_[i])
      .AddFormatType(GetSurfaceFormat())
      .Create();
  }
  return views;
}

auto gdm::vk::Renderer::CreateCommandPool(bool reset_on_begin, uint queue_id, VkCommandPoolCreateFlagBits flags) -> VkCommandPool
{
  VkCommandPoolCreateInfo cmd_pool_create_info = {};
  cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_create_info.queueFamilyIndex = queue_id;
  cmd_pool_create_info.flags = flags;

  if (reset_on_begin)
    cmd_pool_create_info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  VkCommandPool pool;
  VkResult res = vkCreateCommandPool(*device_, &cmd_pool_create_info, &allocator_, &pool);
  ASSERTF(res == VK_SUCCESS, "vkCreateCommandPool error %d\n", res);

  return pool;
}

auto gdm::vk::Renderer::CreateCommandBuffers(VkCommandPool pool, uint count) -> std::vector<VkCommandBuffer>
{
  VkCommandBufferAllocateInfo cmd_buf_alloc_info = {};
  cmd_buf_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd_buf_alloc_info.commandPool = pool;
  cmd_buf_alloc_info.commandBufferCount = count;
  cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

  std::vector<VkCommandBuffer> buffer(count);
  VkResult res = vkAllocateCommandBuffers(*device_, &cmd_buf_alloc_info, &buffer[0]);
  ASSERTF(res == VK_SUCCESS, "vkAllocateCommandBuffers error %d\n", res);

  return buffer;
}

auto gdm::vk::Renderer::CreateDescriptorPool(int max_sets, const std::vector<std::pair<gfx::EResourceType, uint>>& pools_description) -> VkDescriptorPool
{
  std::vector<VkDescriptorPoolSize> pools;

  for(const auto& [type,size] : pools_description)
  {
    VkDescriptorPoolSize pool = {};
    pool.type = static_cast<VkDescriptorType>(type);
    pool.descriptorCount = static_cast<uint>(size);
    pools.push_back(pool);
  }

  VkDescriptorPoolCreateInfo pool_create_info = {}; 
  pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  pool_create_info.maxSets = max_sets;
  pool_create_info.poolSizeCount = static_cast<uint>(pools.size());
  pool_create_info.pPoolSizes = pools.data();
  pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    
  VkDescriptorPool descriptor_pool;
  VkResult res = vkCreateDescriptorPool(*device_, &pool_create_info, NULL, &descriptor_pool);
  ASSERTF(res == VK_SUCCESS, "vkCreateDescriptorPool failed %d", res);

  return descriptor_pool;
}

auto gdm::vk::Renderer::CreateDebugCallbackLegacy() -> VkDebugReportCallbackEXT
{
  static VkDebugReportCallbackEXT callback = nullptr;
  
  if constexpr (gfx::v_DebugBuild)
  {
      PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
      CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");

      // PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
      // DestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");

      if (!CreateDebugReportCallback)
        return nullptr;

      VkDebugReportCallbackCreateInfoEXT cb_create_info {};
      cb_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
      cb_create_info.pNext = nullptr;
      cb_create_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
                             VK_DEBUG_REPORT_ERROR_BIT_EXT |
                             VK_DEBUG_REPORT_WARNING_BIT_EXT |
                             VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
      cb_create_info.pfnCallback = _private::DebugReportLegacyCB;
      cb_create_info.pUserData = nullptr;


      VkResult res = CreateDebugReportCallback(instance_, &cb_create_info, &allocator_, &callback);
      ASSERTF(res == VK_SUCCESS, "vkCreateDebugReportCallbackEXT error %d\n", res);
  }
  return callback;
}

// https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf

auto gdm::vk::Renderer::CreateDebugCallbackModern() -> VkDebugUtilsMessengerEXT
{
  static VkDebugUtilsMessengerEXT callback = nullptr;
  
  if constexpr (gfx::v_DebugBuild)
  {
      PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessenger;
      CreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");

      // PFN_vkDestroyDebugUtilsMessengerEXT DestroyDebugUtilsMessenger;
      // DestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");

      if (!CreateDebugUtilsMessenger)
        return nullptr;

      VkDebugUtilsMessengerCreateInfoEXT cb_create_info {};
      cb_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      cb_create_info.pNext = nullptr;
      cb_create_info.flags = 0;
      cb_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      cb_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
      cb_create_info.pfnUserCallback = _private::DebugReportModernCB;
      cb_create_info.pUserData = nullptr;

      VkResult res = CreateDebugUtilsMessenger(instance_, &cb_create_info, &allocator_, &callback);
      ASSERTF(res == VK_SUCCESS, "vkCreateDebugUtilsMessengerEXT error %d\n", res);
  }
  return callback;
}

auto gdm::vk::Renderer::FillInstanceLayersInfo() -> std::vector<const char*>
{
  std::vector<const char*> instance_layers;

  if constexpr (gfx::v_DebugBuild)
  {
    instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
    instance_layers.push_back("VK_LAYER_KHRONOS_validation");
    
    helpers::RemoveUnsupportedLayers(instance_layers);

    ASSERT(!instance_layers.empty());
  }

  return instance_layers;
}

auto gdm::vk::Renderer::FillInstanceExtensionsInfo() -> std::vector<const char*>
{
  std::vector<const char*> instance_extensions;

  instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);  // for dynamic_indexing

  if constexpr (gfx::v_DebugBuild)
  {
    instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  if constexpr (gfx::v_Windows)
    instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

  auto unsupported = helpers::FindUnsupportedInstanceExtensions(instance_extensions);

  if (!unsupported.empty())
  {
    for (auto&& num : unsupported)
    {
      ENSUREF(false, "Unsupported extensions: %s", instance_extensions[num]);
      auto it = instance_extensions.begin() + num;
      instance_extensions.erase(it);
    }
  }

  return instance_extensions;
}

void gdm::vk::Renderer::Cleanup()
{
  for (auto view : present_images_views_)
    vkDestroyImageView(*device_, *view, &allocator_);
  for (auto command_buffer : frame_command_buffers_)
    vkResetCommandBuffer(command_buffer, {});

  vkDestroyCommandPool(*device_, setup_command_pool_, &allocator_);
  vkDestroySwapchainKHR(*device_, swapchain_, &allocator_);
  vkDestroySurfaceKHR(instance_, surface_, &allocator_);

  GMDelete(device_);

  vkDestroyInstance(instance_, &allocator_);
}

// --helpers

auto gdm::vk::helpers::EnumerateInstanceExtensionsProps() -> std::vector<VkExtensionProperties>
{
  uint num = 0;
  VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &num, nullptr);
  ASSERTF(res == VK_SUCCESS, "vkEnumerateInstanceExtensionProperties error %d\n", res);
  
  std::vector<VkExtensionProperties> props(num);
  res = vkEnumerateInstanceExtensionProperties(nullptr, &num, &props[0]);
  ASSERTF(res == VK_SUCCESS, "vkEnumerateInstanceExtensionProperties error %d\n", res);

  return props;
}

auto gdm::vk::helpers::FindUnsupportedLayers(const std::vector<const char*>& layers_info) -> std::vector<size_t>
{
  uint32_t layers_count = 0;
  vkEnumerateInstanceLayerProperties(&layers_count, NULL);
  ASSERTF(layers_count != 0, "Failed to find any layer");

  std::vector<VkLayerProperties> layers_available(layers_count);
  VkResult res = vkEnumerateInstanceLayerProperties(&layers_count, layers_available.data());
  ASSERTF(res == VK_SUCCESS, "vkEnumerateInstanceLayerProperties error %d", res);

  std::vector<size_t> unsupported;

  for (size_t i = 0; i < layers_info.size(); ++i)
  {
    bool found = false;
    for (size_t k = 0; k < layers_available.size() && !found; ++k)
      found = strcmp(layers_info[i], layers_available[k].layerName) == 0;
    if (!found)
      unsupported.push_back(i);
  }
  return unsupported;
}

void gdm::vk::helpers::RemoveUnsupportedLayers(std::vector<const char*>& layers_info)
{
  uint32_t layers_count = 0;
  vkEnumerateInstanceLayerProperties(&layers_count, NULL);
  ASSERTF(layers_count != 0, "Failed to find any layer");

  std::vector<VkLayerProperties> layers_available(layers_count);
  VkResult res = vkEnumerateInstanceLayerProperties(&layers_count, layers_available.data());
  ASSERTF(res == VK_SUCCESS, "vkEnumerateInstanceLayerProperties error %d", res);

  int to_remove = 0;

  for (size_t i = 0; i < layers_info.size() - to_remove; ++i)
  {
    bool found = false;
    for (size_t k = 0; k < layers_available.size() && !found; ++k)
      found = strcmp(layers_info[i], layers_available[k].layerName) == 0;
    if (!found)
    {
      std::swap(layers_info[i], layers_info[layers_info.size() - 1 - to_remove]);
      --i;
      ++to_remove;
    }
  }
  layers_info.resize(layers_info.size() - to_remove);
}

auto gdm::vk::helpers::FindUnsupportedInstanceExtensions(const std::vector<const char*>& extensions_info) -> std::vector<size_t>
{
  uint32_t extensions_count = 0;
  vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, NULL);
  ASSERTF(extensions_count != 0, "Failed to find any extensions");

  std::vector<VkExtensionProperties> extensions_available(extensions_count);
  VkResult res = vkEnumerateInstanceExtensionProperties(NULL, &extensions_count, extensions_available.data());
  ASSERTF(res == VK_SUCCESS, "vkEnumerateInstanceExtensionProperties error %d", res);

  std::vector<size_t> unsupported;

  for (size_t i = 0; i < extensions_info.size(); ++i)
  {
    bool found = false;
    for (size_t k = 0; k < extensions_available.size() && !found; ++k)
      found = strcmp(extensions_info[i], extensions_available[k].extensionName) == 0;
    if (!found)
      unsupported.push_back(i);
  }
  return unsupported;
}
