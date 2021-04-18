// *************************************************************
// File:    api.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef GM_GFX_API_H
#define GM_GFX_API_H

#if defined (GFX_DX_API)
#include "dx11/dx_image.h"
#include "dx11/dx_image_view.h"
#include "dx11/dx_device.h"
#include "dx11/dx_sampler.h"
#include "dx11/dx_image_barrier.h"
#include "dx11/dx_buffer_barrier.h"
#include "dx11/dx_semaphore.h"
#include "dx11/dx_fence.h"
#include "dx11/dx_pso.h"
#include "dx11/dx_command_list.h"
#include "dx11/dx_render_pass.h"
#include "dx11/dx_framebuffer.h"
#include "dx11/dx_buffer.h"
#include "dx11/dx_descriptor_set.h"
#elif defined (GFX_VK_API)
#include "vk/vk_image.h"
#include "vk/vk_image_view.h"
#include "vk/vk_device.h"
#include "vk/vk_sampler.h"
#include "vk/vk_image_barrier.h"
#include "vk/vk_buffer_barrier.h"
#include "vk/vk_fence.h"
#include "vk/vk_semaphore.h"
#include "vk/vk_pipeline.h"
#include "vk/vk_command_list.h"
#include "vk/vk_render_pass.h"
#include "vk/vk_framebuffer.h"
#include "vk/vk_buffer.h"
#include "vk/vk_blend_state.h"
#include "vk/vk_descriptor_set.h"
#include "vk/vk_host_allocator.h"
#endif

#endif // GM_GFX_API_H
