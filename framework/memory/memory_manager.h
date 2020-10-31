// *************************************************************
// File:    memory_manager.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MEM_MANAGER_H
#define AH_GDM_MEM_MANAGER_H

#include <cstddef>

#include "memory_tag.h"

using std::size_t;

namespace gdm {

struct MemoryManager
{
  static auto Allocate(size_t bytes, MemoryTagValue tag = 0) -> void*;
  static auto AllocateAligned(size_t bytes, size_t align, MemoryTagValue tag = 0) -> void*;
  static auto Reallocate(void* ptr, size_t new_bytes, MemoryTagValue tag = 0) -> void*;
  static auto ReallocateAligned(void* ptr, size_t new_bytes, size_t align, MemoryTagValue tag = 0) -> void*;
  static void Deallocate(void* ptr, MemoryTagValue tag = 0);
  static void DeallocateAligned(void* ptr, size_t align, MemoryTagValue tag = 0);

public:
  static auto GetPointerSize(void* ptr, size_t align = 0) -> size_t;
  static auto GetTagUsage(MemoryTagValue tag) -> size_t;
  static auto GetTagName(MemoryTagValue tag) -> const char*;

public:
  constexpr static auto GetDefaultAlignment() -> size_t { return alignof(std::max_align_t); }

}; // struct Manager

} // namespace gdm

#endif // AH_GDM_MEM_MANAGER_H