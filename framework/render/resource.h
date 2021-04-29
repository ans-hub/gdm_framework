// *************************************************************
// File:    resource.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GFX_RESOURCE_H
#define AH_GFX_RESOURCE_H

#if defined(GFX_DX_API)
#include "dx11/dx_resource.h"
#elif defined (GFX_VK_API)
#include "vk/vk_resource.h"
#else
#include "ogl/gl_resource.h"
#endif

#endif // AH_GFX_RESOURCE_H
