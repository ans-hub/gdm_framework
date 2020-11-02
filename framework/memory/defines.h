// *************************************************************
// File:    defines.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MEM_DEFINES_H
#define AH_GDM_MEM_DEFINES_H

#include <cstdint>

#include <memory/memory_manager.h>

using std::size_t;
using std::uintptr_t;
using std::intptr_t;
using uint = unsigned int;

#define KB(x) x * 1024
#define MB(x) x * 1024 * 1024
#define GB(x) x * 1024 * 1024 * 1024

#define GMNew new
#define GMDelete(ptr) delete(ptr)
#define GMNewTracked(tag) new(tag)
#define GMDeleteTracked(tag, ptr) gdm::DeleteTracked(ptr, tag)

#endif // AH_GDM_MEM_DEFINES_H