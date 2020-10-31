// *************************************************************
// File:    helpers.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MEM_HELPERS_H
#define AH_GDM_MEM_HELPERS_H

#include "defines.h"

namespace gdm::mem {

  auto PtrToUptr(void* ptr) -> uintptr_t;
  auto UptrToPtr(uintptr_t uptr) -> void*;
  bool IsAligned(void* ptr, size_t align);
  auto AlignAddress(uintptr_t address, size_t alignment) -> uintptr_t;
  auto AlignAddress(void* ptr, size_t alignment) -> void*;

} // namespace gdm::mem

#endif // AH_GDM_MEM_HELPERS_H