// *************************************************************
// File:    scoped_locks.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_SCOPED_LOCKS_H
#define AH_GDM_SCOPED_LOCKS_H

#include <mutex>

#include "system/profiler.h"

#define GDM_UNIQUE_LOCK(mx, name, color)\
  CPU_PROFILE_ENTER("Mutex", name, color);\
  std::unique_lock<decltype(mx)> GDM_CONCAT(lock,__LINE__)(mx);\
  CPU_PROFILE_LEAVE();

#define GDM_UNIQUE_LOCK_VAR(mx, varname, name, color)\
  CPU_PROFILE_ENTER("Mutex", name, color);\
  std::unique_lock<decltype(mx)> varname(mx);\
  CPU_PROFILE_LEAVE();

#define GDM_TRY_LOCK_FOR(mcsec, bool_var, mx, name, color)\
  CPU_PROFILE_ENTER("Mutex", name, color);\
  std::unique_lock<std::timed_mutex> GDM_CONCAT(lock,__LINE__)(mx, std::defer_lock);\
  if (!GDM_CONCAT(lock,__LINE__).try_lock_for(std::chrono::microseconds(mcsec))){\
    CPU_PROFILE_LEAVE();\
    bool_var = false;\
  }\
  else{\
    CPU_PROFILE_LEAVE();\
    bool_var = true;\
  }\

#define GDM_LOCK_GUARD(mx, name, color)\
  CPU_PROFILE_ENTER("Mutex", name, color);\
  std::lock_guard<std::mutex> GDM_CONCAT(lock,__LINE__)(mx);\
  CPU_PROFILE_LEAVE();

#define GDM_LOCK_GUARD_VAR(mx, varname, name, color)\
  CPU_PROFILE_ENTER("Mutex", name, color);\
  std::lock_guard<std::mutex> varname(mx);\
  CPU_PROFILE_LEAVE();

#define GDM_COND_VAR(varmx, varcv, name, color, cond)\
  CPU_PROFILE_ENTER("Mutex", name, color);\
  std::unique_lock<std::mutex> lock(varmx);\
  varcv.wait(lock, [&](){ return cond; });\
  CPU_PROFILE_LEAVE();

#endif // AH_GDM_SCOPED_LOCKS_H
