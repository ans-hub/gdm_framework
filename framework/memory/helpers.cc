// *************************************************************
// File:    helpers.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "helpers.h"

#include "system/assert_utils.h"

uintptr_t gdm::mem::PtrToUptr(void* ptr)
{
  uintptr_t uptr = reinterpret_cast<uintptr_t>(ptr);
  return uptr;
}

void* gdm::mem::UptrToPtr(uintptr_t uptr)
{
  void* ptr = reinterpret_cast<void*>(uptr);
  return ptr;
}

bool gdm::mem::IsAligned(void* ptr, size_t align)
{
  uintptr_t uptr = PtrToUptr(ptr);
  int reminder = static_cast<int>(uptr % align);
  return reminder == 0;
}

auto gdm::mem::AlignAddress(uintptr_t address, size_t alignment) -> uintptr_t
{
  const size_t mask = alignment - 1;
  bool is_align_pow2 = (alignment & mask) == 0;

  ASSERTF(is_align_pow2, "Alignment is not power of 2");

  uintptr_t result = (address + mask) & ~mask;
  return result;
}

auto gdm::mem::AlignAddress(void* ptr, size_t alignment) -> void*
{
  uintptr_t address = PtrToUptr(ptr);
  uintptr_t aligned_address = AlignAddress(address, alignment);
  return UptrToPtr(aligned_address);
}
