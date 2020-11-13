// *************************************************************
// File:    defines.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DEFINES_H
#define AH_DEFINES_H

#ifndef GFX_VK_API
#define GFX_VK_API
#endif

#if defined (GFX_DX_API)
#include "dx11/dx_defines.h"
#elif defined (GFX_VK_API)
#include "vk/vk_defines.h"
#else
#include "gl/gl_defines.h"
#endif

#if defined (GFX_DX_API)
namespace gdm {
  namespace dx {}
  namespace api = dx;
}
#elif defined (GFX_VK_API)
namespace gdm {
  namespace vk {}
  namespace api = vk;
}
#else
namespace gdm {
  namespace dx {}
  namespace api = gl;
}
#endif

namespace gdm::gfx {

  using Offsets = std::vector<uint>;

  enum EQueueType : uint
  {
    GRAPHICS,
    COMPUTE,
    TRANSFER,
    PRESENTATION
  };

  using QueueType = uint;

  enum EUboType : uint
  {
    PER_FRAME,
    PER_OBJECT,
  };

  using UboType = uint;

  enum EShaderType : uint
  {
    VX,
    PX,
    CS,
    GS,
    Size
  };

  using ShaderType = uint;

  enum EResourceType : uint
  {
    UNIFORM_BUFFER = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    UNIFORM_DYNAMIC = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    SAMPLED_IMAGE = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    SAMPLER = VK_DESCRIPTOR_TYPE_SAMPLER,
    COMBINED_SAMPLER = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER

  }; // enum EResourceType

  using ResourceType = uint;

  enum EShaderStage : uint
  {
    VERTEX_STAGE = VK_SHADER_STAGE_VERTEX_BIT,
    FRAGMENT_STAGE = VK_SHADER_STAGE_FRAGMENT_BIT

  }; // enum EShaderStage

  using ShaderStage = uint;

  enum EDeviceProps : uint
  {
    VSYNC = 1 << 1,
    DEBUG_DEVICE = 1 << 2,
    PROFILE_MARKS = 1 << 3

  }; // enum EDeviceProps

  using DeviceProps = uint;

  enum EState : uint
  {
    ON,
    OFF

  }; // enum EState

  using State = uint;

  // todo: remove
  enum EPassFlags : uint
  {
    RS_STATE = 1 << 1,
    RS_VIEWPORT = 1 << 2,
    CLEAR_SHADER_RES = 1 << 3,
    NULL_VBUFF = 1 << 4,
    NULL_IBUFF = 1 << 5,
    DONT_CLEAR_RTV = 1 << 6,
    DONT_CLEAR_DSV = 1 << 7

  }; // enum EPassFlags

  using PassFlags = uint;

} // namespace gdm::gfx

#endif // AH_DEFINES_H
