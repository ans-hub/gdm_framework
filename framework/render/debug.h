// *************************************************************
// File:    debug.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DEBUG_H
#define AH_DEBUG_H

#include "defines.h"

#include "vk/vk_debug_interface.h"  // todo: remove VK namespace file from here and make separate file api_debug.h to avoid chain deps

#include <math/vector4.h>
#include <system/diff_utils.h>

namespace gdm::gfx {

  struct DebugScope
  {
    DebugScope(VkCommandBuffer cmd, const char* name, const Vec4f& color)
      : cmd_{cmd}
    {
      api::debug::BeginDebugLabel(cmd_, name, color);
    }
    ~DebugScope() { api::debug::EndDebugLabel(cmd_); }
    VkCommandBuffer cmd_;
  };

#ifdef GDM_LABELS_ENABLED

# define GDM_LABEL_S(col)\
    [[maybe_unused]] ::gdm::gfx::DebugScope GDM_CONCAT(label, __LINE__)(cmd, v_eventname64, col);

# define GDM_LABEL_B(col)\
    do {\
      ::gdm::api::debug::BeginDebugLabel(cmd, v_eventname64, col);\
    } while(0);
# define GDM_LABEL_E()\
    do {\
      ::gdm::api::debug::EndDebugLabel(cmd);\
    } while(0);
# define GDM_LABEL_I(col)\
    do {\
      ::gdm::api::debug::InsertDebugLabel(cmd, v_eventname64, col);\
    } while(0);

#else
# define GDM_LABEL_B(col)
# define GDM_LABEL_E()
# define GDM_LABEL_I(col)
#endif

} // namespace gdm::gfx

#endif // AH_DEBUG_H