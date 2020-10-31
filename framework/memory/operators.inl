// *************************************************************
// File:    operators.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "memory/operators.h"

template <class T>
void gdm::DeleteTracked(T* mem, gdm::MemoryTagValue tag) noexcept
{
  if (!mem)
    return;
  mem->~T();
  ::operator delete(mem, sizeof(T), tag);
}
