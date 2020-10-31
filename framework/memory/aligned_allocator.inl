// *************************************************************
// File:    aligned_allocator.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "memory/aligned_allocator.h"

#include "memory/memory_manager.h"

// --public

template <class T, size_t Align>
gdm::AlignedAllocator<T,Align>::AlignedAllocator(int memory_tag)
  : memory_tag_{memory_tag}
{ }

// --public stl

template <class T, size_t Align>
auto gdm::AlignedAllocator<T,Align>::allocate(size_t num)
{
  void* ptr = MemoryManager::AllocateAligned(sizeof(T) * num, Align, memory_tag_);
  return static_cast<T*>(ptr);
}

template <class T, size_t Align>
void gdm::AlignedAllocator<T,Align>::deallocate(T* ptr, size_t)
{
  MemoryManager::DeallocateAligned(ptr, Align, memory_tag_);
}

template<class T1, class T2, size_t Align>
constexpr bool operator==(
  const gdm::AlignedAllocator<T1,Align>&, const gdm::AlignedAllocator<T2,Align>&) noexcept
{
  return true;
}

template<class T1, class T2, size_t Align>
constexpr bool operator!=(
  const gdm::AlignedAllocator<T1,Align>&, const gdm::AlignedAllocator<T2,Align>&) noexcept
{
  return false;
}
