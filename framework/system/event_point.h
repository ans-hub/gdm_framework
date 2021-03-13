// *************************************************************
// File:    event_point.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_EVENT_POINT_H
#define AH_GDM_EVENT_POINT_H

#ifdef GDM_PROFILER_ENABLED
#include <3rdparty/microprofile/microprofile.h>
#endif

#ifdef GDM_LOGGER_ENABLED
#include "logger.h"
#endif

// Event points consists of profiler markers ans log markers.
// Necessary to combine logging and profiling functionality in
// critical code. You may add own markers in GDM_EVENT_POINT body 
// from within other projects

// Usage:
//  GDM_EVENT_POINT("OnlyLog", GDM_LOG("Name custom"));
//  GDM_EVENT_POINT("LogGpu and log", GDM_CPU(core::COLOR_BLUE) GDM_LOG_E());
//  GDM_EVENT_POINT("RenderPass", GDM_LABEL_B()) // GDM_LABEL_B from other project

// Profiler markers for event points

#ifdef GDM_PROFILER_ENABLED
  
# define GDM_CPU(color)\
    CPU_PROFILE_ENTER("CPUGroup", v_eventname64, color);\
    CPU_PROFILE_GUARD(eventname64);
 
# define GDM_CPU_G(group, color)\
    CPU_PROFILE_ENTER(group, v_eventname64, color);\
    CPU_PROFILE_GUARD(v_eventname64);

# define GDM_GPU(color)\
    GPU_PROFILE_ENTER("GPUGroup", v_eventname64, color);\
    GPU_PROFILE_GUARD(v_eventname64);

# define GDM_GPUG(group, color)\
    GPU_PROFILE_ENTER(group, v_eventname64, color);\
    GPU_PROFILE_GUARD(v_eventname64);
#else
# define GDM_CPU(...)
# define GDM_CPU_G(...)
# define GDM_GPU(...)
# define GDM_GPU_G(...)
#endif

// Logging markers for event point

#ifdef GDM_LOGGER_ENABLED

# define GDM_LOG(...)\
    do {\
      char log_str[256] = {"%s"};\
      snprintf(&log_str[2], 254, __VA_ARGS__);\
      gdm::Logger::LogThread(log_str, "");\
    } while(0);

# define GDM_LOG_E()\
    do {\
      char log_str[256];\
      snprintf(log_str, 254, "%s\n", v_eventname64);\
      gdm::Logger::LogThread(log_str, "");\
    } while(0);

#else
# define GDM_LOG(...)
# define GDM_LOG_E()
#endif

// Event points by itself

#if defined(GDM_LOGGER_ENABLED) || defined(GDM_PROFILER_ENABLED) || defined(GDM_LABELS_ENABLED)
# define GDM_EVENT_POINT_NEXT(...) __VA_ARGS__
# define GDM_EVENT_POINT(name, ...) char v_eventname64[64] = name; GDM_EVENT_POINT_NEXT(__VA_ARGS__)
#else
# define GDM_EVENT_POINT(...)
#endif

// Helper macroses

#if defined(GDM_LOGGER_ENABLED) || defined(GDM_PROFILER_ENABLED) || defined(GDM_LABELS_ENABLED)
# define GDM_EVENT_STR v_eventname64
#else
# define GDM_EVENT_STR
#endif

#endif // AH_GDM_EVENT_POINT_H
