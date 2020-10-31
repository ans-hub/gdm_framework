// *************************************************************
// File:    memory_manager.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "memory_manager.h"

#include <memory/memory_tracker.h>
#include <memory/defines.h>
#include <math/general.h>
#include <system/assert_utils.h>

#if defined (_WIN32)
#include <malloc.h>
#else
#include <cstdlib>
#endif

// --public

void* gdm::MemoryManager::Allocate(size_t bytes, MemoryTagValue tag)
{
  return AllocateAligned(bytes, GetDefaultAlignment(), tag); 
}

void* gdm::MemoryManager::AllocateAligned(size_t bytes, size_t align, MemoryTagValue tag)
{  
  align = math::Max(align, GetDefaultAlignment());
  ASSERTF(math::IsPowerOfTwo(align), "Alignment %zu is not power of 2", align);

#if defined (_WIN32)
  void* ptr = _aligned_malloc(bytes, align);
#else
  void* ptr = std::aligned_alloc(size, align);
#endif

  MemoryTracker::GetInstance().AddUsage(tag, GetPointerSize(ptr, align));
  return ptr;
}

void* gdm::MemoryManager::Reallocate(void* ptr, size_t new_bytes, MemoryTagValue tag)
{
  return ReallocateAligned(ptr, new_bytes, GetDefaultAlignment(), tag);
}

void* gdm::MemoryManager::ReallocateAligned(void* ptr, size_t new_bytes, size_t align, MemoryTagValue tag)
{
  align = math::Max(align, GetDefaultAlignment());
  ASSERTF(math::IsPowerOfTwo(align), "Alignment %zu is not power of 2", align);

#if defined (_WIN32)
  MemoryTracker::GetInstance().SubUsage(tag, GetPointerSize(ptr, align));
  void* new_ptr = _aligned_realloc(ptr, new_bytes, align);
  MemoryTracker::GetInstance().AddUsage(tag, GetPointerSize(new_ptr, align));
#else
  void* new_ptr = Allocate(new_bytes, align);
  memcpy(new_mem, ptr, GetPointerSize(ptr, align));
  Deallocate(ptr, align);
#endif

  MemoryTracker::GetInstance().AddUsage(tag, GetPointerSize(new_ptr, align));
  return new_ptr;
}

void gdm::MemoryManager::Deallocate(void* ptr, MemoryTagValue tag)
{
  DeallocateAligned(ptr, GetDefaultAlignment(), tag);
}

void gdm::MemoryManager::DeallocateAligned(void* ptr, size_t align, MemoryTagValue tag)
{
  MemoryTracker::GetInstance().SubUsage(tag, GetPointerSize(ptr, align));
#if defined (_WIN32)
  _aligned_free(ptr);
#else
  std::free(mem);
#endif
}

size_t gdm::MemoryManager::GetPointerSize(void* ptr, size_t align)
{
  if (!ptr)
    return 0;

  align = math::Max(align, GetDefaultAlignment());
  ASSERTF(math::IsPowerOfTwo(align), "Alignment %zu is not power of 2", align);

#if defined(_WIN32)
  return _aligned_msize(ptr, align, 0);
#else
  return malloc_usabe_size(ptr);
#endif
}

auto gdm::MemoryManager::GetTagUsage(MemoryTagValue tag) -> size_t
{
  return MemoryTracker::GetInstance().GetTagUsage(tag);
}

auto gdm::MemoryManager::GetTagName(MemoryTagValue tag) -> const char*
{
  return MemoryTracker::GetInstance().GetTagName(tag);
}
