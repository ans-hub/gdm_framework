// *************************************************************
// File:    profiler.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_PROFILER_H
#define AH_GDM_PROFILER_H

#ifdef GDM_PROFILER_ENABLED
#include <3rdparty/microprofile/microprofile.h>
#endif

namespace gdm {

namespace core {

  enum EProfilerColors : unsigned   // from http://fugal.net/vim/rgbtxt.html
  {
    COLOR_AUTO = 0x0,
	  COLOR_MEDIUMAQUAMARINE = 0x66cdaa,
	  COLOR_MEDIUMTURQUOISE = 0x48d1cc,
	  COLOR_STEELBLUE = 0x4682b4,
	  COLOR_FORESTGREEN = 0x228b22,
	  COLOR_SEAGREEN = 0x2e8b57,
	  COLOR_LIMEGREEN = 0x32cd32,
	  COLOR_DARKGOLDENROD = 0xb8860b,
	  COLOR_SADDLEBROWN = 0x8b4513,
	  COLOR_PERU = 0xcd853f,
	  COLOR_DARKORANGE = 0xff8c00,
	  COLOR_HOTPINK = 0xff69b4,
	  COLOR_DEEPPINK = 0xff1493,
	  COLOR_DARKVIOLET = 0x9400d3,
	  COLOR_STEELBLUE2 = 0x5cacee,
	  COLOR_PALEGREEN2 = 0x90ee90,
	  COLOR_INDIANRED1 = 0xff6a6a,
	  COLOR_INDIANRED2 = 0xee6363,
	  COLOR_INDIANRED3 = 0xcd5555,
	  COLOR_INDIANRED4 = 0x8b3a3a,
	  COLOR_DARKGREY = 0xa9a9a9,
    COLOR_BLACK = 0x010101,
	  COLOR_WHITESMOKE = 0xf5f5f5

  }; // enum EProfilerColors

  using ProfilerColors = unsigned;

} // namespace core

// Peroforms profiling functionality

#ifndef GDM_CONCAT
#define GDM_CONCAT_(x, y) x##y
#define GDM_CONCAT(x, y) GDM_CONCAT_(x, y)
#endif

#ifdef GDM_PROFILER_ENABLED

namespace core {

  struct CpuProfilerScopeGuard { ~CpuProfilerScopeGuard(); };
  struct GpuProfilerScopeGuard { ~GpuProfilerScopeGuard(); };

} // namespace core

#  define GDM_PROFILING_ENABLE()\
  do {\
    MicroProfileSetEnableAllGroups(true);\
    MicroProfileSetForceMetaCounters(true);\
    MicroProfileStartContextSwitchTrace();\
  } while(0)

#define GDM_PROFILE_THIS_THREAD_ENABLE(name) MicroProfileOnThreadCreate(name)
#define GDM_PROFILER_SHUTDOWN() MicroProfileShutdown()
#define GDM_PROFILER_FRAME(...) MicroProfileFlip(nullptr)

#define CPU_PROFILE_SCOPE(group, name, color) MICROPROFILE_SCOPEI(group, name, color)
#define CPU_PROFILE_ENTER(group, name, color) MICROPROFILE_ENTERI(group, name, color)
#define CPU_PROFILE_LEAVE() MICROPROFILE_LEAVE()
#define CPU_PROFILE_GUARD(name) core::CpuProfilerScopeGuard GDM_CONCAT(cpu_guard,name) {}

#define GPU_PROFILE_ENTER(...)
#define GPU_PROFILE_LEAVE()
#define GPU_PROFILE_GUARD(...)

inline core::CpuProfilerScopeGuard::~CpuProfilerScopeGuard() { CPU_PROFILE_LEAVE(); }
// inline core::GpuProfilerScopeGuard::~GpuProfilerScopeGuard() { GPU_PROFILE_LEAVE(); }

#else	// !GDM_PROFILER_ENABLED

#define GDM_PROFILING_ENABLE()
#define GDM_PROFILE_THIS_THREAD_ENABLE(...)
#define GDM_PROFILER_SHUTDOWN()
#define GDM_PROFILER_FRAME(...)

#define CPU_PROFILE_SCOPE(...)
#define CPU_PROFILE_ENTER(...)
#define CPU_PROFILE_LEAVE()
#define CPU_PROFILE_GUARD(...)

#define GPU_PROFILE_ENTER(...)
#define GPU_PROFILE_LEAVE()
#define GPU_PROFILE_GUARD(...)

#endif

} // namespace gdm

#endif // AH_GDM_PROFILER_H
