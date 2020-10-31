// *************************************************************
// File:    aligned_allocator.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_ALLOCATOR_H
#define AH_GDM_ALLOCATOR_H

#include "memory/memory_manager.h"

namespace gdm {

template <class T, size_t Align = MemoryManager::GetDefaultAlignment()>
struct AlignedAllocator
{
  AlignedAllocator() = default;
  AlignedAllocator(int memory_tag);
  template<class U, size_t Align>
  AlignedAllocator(const AlignedAllocator<U, Align>&) noexcept { }

private:
  int memory_tag_ = 0;
  int align_ = Align;

  // stl stuff

public:
  using value_type = T;

  template <class U>
  struct rebind
  {
    using other = AlignedAllocator<U, Align>;
  };

  auto allocate(size_t num);
  void deallocate(T* ptr, size_t);

}; // struct AlignedAllocator

} // namespace gdm

#include "memory/aligned_allocator.inl"

#endif // AH_GDM_ALLOCATOR_H