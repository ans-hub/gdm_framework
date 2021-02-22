// *************************************************************
// File:    frame_allocator.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "frame_allocator.h"

#include "math/general.h"
#include "system/assert_utils.h"
#include "memory/helpers.h"

// --private

template <size_t Size>
char gdm::FrameAllocator<Size>::buffer_[Size] = {};

template <size_t Size>
size_t gdm::FrameAllocator<Size>::offset_ = 0;

template <size_t Size>
uintptr_t gdm::FrameAllocator<Size>::last_alloc_ = 0;

// --public

template <size_t Size>
template <class T>
void* gdm::FrameAllocator<Size>::Allocate(size_t count)
{
  return Allocate<T>(count, MemoryManager::GetDefaultAlignment());
}

template <size_t Size>
template <class T>
void* gdm::FrameAllocator<Size>::Allocate(size_t count, size_t alignment)
{
  ASSERTF(math::IsPowerOfTwo(alignment), "Alignment %zu is not power of 2", alignment);
  

  size_t size = sizeof(T) * count;
  void* raw = &buffer_[offset_];
  uintptr_t uraw = mem::PtrToUptr(raw);
  size_t mask = alignment - 1;
  uintptr_t uptr = (uraw + mask) & ~mask;

  offset_ += size + (uptr - uraw);
  
  if (offset_ >= Size)
    return nullptr;

  last_alloc_ = uptr;
  return mem::UptrToPtr(uptr);
}

template <size_t Size>
void gdm::FrameAllocator<Size>::Deallocate()
{ }

template <size_t Size>
void gdm::FrameAllocator<Size>::Reset()
{
  offset_ = 0;
  memset(buffer_, 0, Size);
}

template <size_t Size>
size_t gdm::FrameAllocator<Size>::GetFreeSize()
{
  return Size - offset_;
}

template<size_t T1, size_t T2>
constexpr bool operator==(const gdm::FrameAllocator<T1>&, const gdm::FrameAllocator<T2>&) noexcept
{
  return true;
}

template<size_t T1, size_t T2>
constexpr bool operator!=(const gdm::FrameAllocator<T1>&, const gdm::FrameAllocator<T2>&) noexcept
{
  return false;
}

// --stl

template <class T, size_t Size>
auto gdm::FrameAllocatorTyped<T, Size>::allocate(size_t num)
{
  // TODO: what about shared states, interchangable and so on?

  void* ptr = BaseAllocator::template Allocate<T>(sizeof(T) * num);
  uintptr_t uptr = mem::PtrToUptr(ptr);
  return static_cast<T*>(ptr);
}

template <class T, size_t Size>
void gdm::FrameAllocatorTyped<T, Size>::deallocate(T* ptr, size_t)
{
  // TODO: track last ptr (marker) and deallocate if needed
}
