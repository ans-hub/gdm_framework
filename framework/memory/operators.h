// *************************************************************
// File:    operators.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GD_MEM_OPERATORS
#define AH_GD_MEM_OPERATORS

#include "memory/memory_tag.h"

void* operator new(std::size_t size, gdm::MemoryTagValue tag) noexcept;
void operator delete(void* ptr, std::size_t size, gdm::MemoryTagValue tag) noexcept;

namespace gdm {

  template <class T>
  void DeleteTracked(T* mem, gdm::MemoryTagValue tag) noexcept;

} // namespace gdm

#include "operators.inl"

#endif // AH_GD_MEM_OPERATORS