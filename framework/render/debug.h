// *************************************************************
// File:    debug.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DEBUG_H
#define AH_DEBUG_H

#include "defines.h"
#include "api.h"
#include "renderer.h"
#include <math/vector4.h>

// Gpu labels for event point. By convention in caller context should present
// interface to renderer pointer named rdr_ and command list called cmd. Otherwise
// use macroses accept renderer and command list in arguments

namespace gdm::gfx {

  struct DebugScope
  {
    DebugScope(api::Renderer& rdr, api::CommandList& cmd, const char* name, const Vec4f& color)
      : cmd_{cmd}
      , rdr_{rdr}
    {
      rdr_.BeginDebugLabel(cmd_, name, color);
    }
    ~DebugScope() { rdr_.EndDebugLabel(cmd_); }
    api::Renderer& rdr_;
    api::CommandList& cmd_;
  };

#ifdef GDM_LABELS_ENABLED

# define GDM_LABEL_S(col)\
    ::gdm::gfx::DebugScope(*rdr_, cmd, v_eventname64, col);

# define GDM_LABEL_B(col)\
    do {\
      rdr_->BeginDebugLabel(cmd, v_eventname64, col);\
    } while(0);
# define GDM_LABEL_E()\
    do {\
      rdr_->EndDebugLabel(cmd);\
    } while(0);
# define GDM_LABEL_I(col)\
    do {\
      rdr_->InsertDebugLabel(cmd, v_eventname64, col);\
    } while(0);

#else
# define GDM_LABEL_B(col)
# define GDM_LABEL_E()
# define GDM_LABEL_I(col)
#endif

} // namespace gdm::gfx

#endif // AH_DEBUG_H