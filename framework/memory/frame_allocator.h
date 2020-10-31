// *************************************************************
// File:    frame_allocator.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_FRAMEMEM_H
#define AH_GDM_FRAMEMEM_H

#include "memory/memory_manager.h"

namespace gdm {

template <size_t Size>
struct FrameAllocator
{
  FrameAllocator() = default;

  template <class T>
  static auto Allocate(size_t count) -> void*;
  template <class T>
  static auto Allocate(size_t count, size_t alignment) -> void*;

  static void Deallocate();
  static void Reset();
  static auto GetFreeSize() -> size_t;

 private:
  static char buffer_[Size];
  static size_t offset_;
  static uintptr_t last_alloc_;

}; // struct FrameAllocator

template <class T, size_t Size>
struct FrameAllocatorTyped
{
  using value_type = T;
  using BaseAllocator = ::gdm::FrameAllocator<Size>;

  FrameAllocatorTyped() = default;
  template<class U, size_t Size>
  FrameAllocatorTyped(const FrameAllocatorTyped<U,Size>&) noexcept { }

  // stl stuff

  template <class U>
  struct rebind
  {
    using other = FrameAllocatorTyped<U, Size>;
  };

  auto allocate(size_t num);
  void deallocate(T* ptr, size_t);
};

} // namespace gdm

#include "memory/frame_allocator.inl"

#endif // AH_GDM_FRAMEMEM_H