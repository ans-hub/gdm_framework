// *************************************************************
// File:    vk_defines.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_VK_DEFINES_H
#define GM_VK_DEFINES_H

#ifdef _WIN32
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkan/vulkan.hpp"
#include "vulkan/vk_sdk_platform.h"
#else
#define VK_USE_PLATFORM_XCB_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#endif

using uint = unsigned int;
using std::size_t;

namespace gdm::gfx {

#ifdef NDEBUG
constexpr const bool v_DebugBuild = false;
#else
constexpr const bool v_DebugBuild = true;
#endif

#ifdef _WIN32

constexpr const bool v_Windows = true;

#ifndef VK_PROTOTYPES
#define VK_PROTOTYPES
#endif

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#else
constexpr const bool v_Windows = false;
#endif

constexpr inline static uint v_num_images = 2;

enum EFormatType : uint
{
  F1 = VK_FORMAT_R32_SFLOAT,
  F2 = VK_FORMAT_R32G32_SFLOAT,
  F3 = VK_FORMAT_R32G32B32_SFLOAT,
  F4 = VK_FORMAT_R32G32B32A32_SFLOAT,
  F4HALF = VK_FORMAT_R16G16B16A16_SFLOAT,
  SRGB4 = VK_FORMAT_R8G8B8A8_SRGB,
  UNORM4 = VK_FORMAT_R8G8B8A8_UNORM,
  D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
  D32_SFLOAT_S8_UINT = VK_FORMAT_D32_SFLOAT_S8_UINT,
  D16_UNORM = VK_FORMAT_D16_UNORM
};

using FormatType = uint;

enum ETextureAddressMode : uint
{ 
  WRAP = VK_SAMPLER_ADDRESS_MODE_REPEAT,
  BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
  CLAMP = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE

}; // enum ETextureAddressMode

using TextureAddressMode = uint;

enum ECompareFunc : uint
{ 
  NEVER = VK_COMPARE_OP_NEVER,
  LESS = VK_COMPARE_OP_LESS,
  EQUAL = VK_COMPARE_OP_EQUAL,
  LESS_OR_EQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
  GREATER = VK_COMPARE_OP_GREATER,
  NOT_EQUAL = VK_COMPARE_OP_NOT_EQUAL,
  GREATER_OR_EQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
  ALWAYS = VK_COMPARE_OP_ALWAYS

}; // enum ECompareFunc

using CompareFunc = uint;

enum EPrimitiveType : uint
{
  LINE_LIST = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
  LINE_STRIP = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
  TRIANGLE_LIST = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST

}; // enum PrimitiveType

using PrimitiveType = uint;

enum ECullMode : uint
{
  NONE = VK_CULL_MODE_NONE,
  BACK_FACE = VK_CULL_MODE_BACK_BIT,
  FRONT_FACE = VK_CULL_MODE_FRONT_BIT

}; // enum ECullMode

using CullMode = uint;

enum EFrontFace : uint
{
  CLOCKWISE = VK_FRONT_FACE_CLOCKWISE,
  COUNTER_CLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE

}; // enum EFrontFace

using FrontFace = uint;

enum EFillMode : uint
{
  SOLID = VK_POLYGON_MODE_FILL,
  WIREFRAME = VK_POLYGON_MODE_LINE

}; // enum EFillMode

using CullMode = uint;

enum ECommandListFlags : uint
{
  ONCE = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  RENDER_PASS = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
  SIMULTANEOUS = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT

}; // enum ECommandListFlags

using CommandListFlags = uint;

enum ECommandListType : uint
{
  FRAME,
  SETUP

}; // enum CommandListType

using CommandListType = uint;

enum EImageUsage : uint
{
  TRANSFER_SRC_IMG = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
  TRANSFER_DST_IMG = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
  SAMPLED = VK_IMAGE_USAGE_SAMPLED_BIT,
  STORAGE = VK_IMAGE_USAGE_STORAGE_BIT,
  COLOR_ATTACHMENT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
  DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
  TRANSFER_ATTACHMENT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT

}; // enum EImageUsage

using ImageUsage = uint;

enum EImageAspect : uint
{
  ASPECT_COLOR = VK_IMAGE_ASPECT_COLOR_BIT,
  ASPECT_DEPTH = VK_IMAGE_ASPECT_DEPTH_BIT,
  ASPECT_STENCIL = VK_IMAGE_ASPECT_STENCIL_BIT

}; // enum EImageAspect

using ImageAspect = EImageAspect;

enum EFenceFlags : uint
{
  SIGNALED = VK_FENCE_CREATE_SIGNALED_BIT

}; // enum EFenceFlags

using FenceFlags = uint;

using ESemaphoreFlags = EFenceFlags;
using SemaphoreFlags = uint;

enum EImageLayout : uint  // OPTIMAL - non transparent optimal layout for particular hw, LINEAR - linear
{
  UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED,
  PREINITIALIZED = VK_IMAGE_LAYOUT_PREINITIALIZED,
  GENERAL = VK_IMAGE_LAYOUT_GENERAL,
  COLOR_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  TRANSFER_SRC_OPTIMAL = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
  TRANSFER_DST_OPTIMAL = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
  DEPTH_STENCIL_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
  DEPTH_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
  STENCIL_ATTACHMENT_OPTIMAL = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
  STENCIL_READ_ONLY_OPTIMAL = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
  PRESENT_SRC = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  SHADER_READ_OPTIMAL = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
};

using ImageLayout = uint;

enum EMemoryType : uint
{
  DEVICE_LOCAL = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  HOST_VISIBLE = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
  HOST_COHERENT = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  HOST_CACHED = VK_MEMORY_PROPERTY_HOST_CACHED_BIT

}; // enum EMemoryType

using MemoryType = uint;

enum EBufferUsage : uint
{
  VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
  INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
  TRANSFER_SRC = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  TRANSFER_DST = VK_BUFFER_USAGE_TRANSFER_DST_BIT

}; // enum EBufferUsage

using BufferUsage = uint;

enum EAccess : uint
{
  INDIRECT_COMMAND_READ = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
  INDEX_READ = VK_ACCESS_INDEX_READ_BIT,
  VERTEX_ATTRIBUTE_READ = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
  UNIFORM_READ = VK_ACCESS_UNIFORM_READ_BIT,
  INPUT_ATTACHMENT_READ = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
  SHADER_READ = VK_ACCESS_SHADER_READ_BIT,
  SHADER_WRITE = VK_ACCESS_SHADER_WRITE_BIT,
  COLOR_ATTACHMENT_READ = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
  COLOR_ATTACHMENT_WRITE = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  DEPTH_STENCIL_ATTACHMENT_READ = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
  DEPTH_STENCIL_ATTACHMENT_WRITE = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
  TRANSFER_READ = VK_ACCESS_TRANSFER_READ_BIT,
  TRANSFER_WRITE = VK_ACCESS_TRANSFER_WRITE_BIT,
  HOST_READ = VK_ACCESS_HOST_READ_BIT,
  HOST_WRITE = VK_ACCESS_HOST_WRITE_BIT,
  MEMORY_READ = VK_ACCESS_MEMORY_READ_BIT,
  MEMORY_WRITE = VK_ACCESS_MEMORY_WRITE_BIT

}; // enum EAccess

using AccessFlags = uint;

enum class EStage : uint
{
  TOP_OF_PIPE = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
  DRAW_INDIRECT = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
  VERTEX_INPUT = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
  VERTEX_SHADER = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
  GEOMETRY = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
  FRAGMENT = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
  EARLY_FRAGMENT = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
  COMPUTE = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
  TRANSFER = VK_PIPELINE_STAGE_TRANSFER_BIT,
  BOTTOM_OF_PIPE = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
  HOST = VK_PIPELINE_STAGE_HOST_BIT,
  ALL_GRAPHICS = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
  ALL_COMMANDS = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT

}; // enum EStage

using Stage = uint;

enum EDynamicState : uint
{
  DS_SCISSOR = VK_DYNAMIC_STATE_SCISSOR,
  DS_VIEWPORT = VK_DYNAMIC_STATE_VIEWPORT

}; // enum EDynamicState

using DynamicState = uint;

enum EBindingFlags : uint
{
  UPDATE_AFTER_BIND = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
  VARIABLE_DESCRIPTOR = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT

}; // enum EBindingFlags

using BindingFlags = uint;

} // namespace gdm::gfx

#endif // GM_VK_DEFINES_H
