// *************************************************************
// File:    memory_tracker.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MEM_TRACKER_H
#define AH_GDM_MEM_TRACKER_H

#include <array>
#include <atomic>

#include "memory_tag_value.h"

namespace gdm {

struct MemoryManager;

struct MemoryTracker
{
  static auto GetInstance() -> MemoryTracker&;

private:
  auto RegisterTag(const char* name) -> size_t;
  void AddUsage(MemoryTagValue tag, size_t bytes);
  void SubUsage(MemoryTagValue tag, size_t bytes);
  auto GetTagUsage(MemoryTagValue tag) -> size_t;
  auto GetTagName(MemoryTagValue tag) -> const char*;

private:
  constexpr static int v_max_tags_ = 32; 
  static std::array<std::atomic<size_t>, v_max_tags_> v_tag_usage_;
  static std::array<const char*, v_max_tags_> v_tag_names_;
  static int count_;

private:
  friend struct MemoryManager;

#ifndef NDEBUG
  template <size_t Value>
  friend struct MemoryTag;
#endif

}; // struct MemoryTracker

} // namespace gdm

#endif // AH_GDM_MEM_TRACKER_H