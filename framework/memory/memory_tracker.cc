// *************************************************************
// File:    memory_tracker.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "memory_tracker.h"

#include <mutex>
#include <cassert>

// --private

std::array<std::atomic<size_t>, gdm::MemoryTracker::v_max_tags_> gdm::MemoryTracker::v_tag_usage_ {};
std::array<const char*, gdm::MemoryTracker::v_max_tags_> gdm::MemoryTracker::v_tag_names_ {};
int gdm::MemoryTracker::count_ {};

// --public 

gdm::MemoryTracker& gdm::MemoryTracker::GetInstance()
{
  static gdm::MemoryTracker s_instance;
  return s_instance;
}

size_t gdm::MemoryTracker::RegisterTag(const char* name)
{
  static size_t untracked_tag = count_++;

  std::unique_lock<std::mutex> lock{};

  assert(count_ < v_max_tags_);
  v_tag_usage_[count_] = 0;
  v_tag_names_[count_] = name;

  return static_cast<size_t>(count_++);
}

void gdm::MemoryTracker::AddUsage(MemoryTagValue tag, size_t bytes)
{
  assert(tag < v_max_tags_);
  v_tag_usage_[tag].fetch_add(bytes);
}

void gdm::MemoryTracker::SubUsage(MemoryTagValue tag, size_t bytes)
{
  assert(tag < v_max_tags_);
  v_tag_usage_[tag].fetch_sub(bytes);
}

size_t gdm::MemoryTracker::GetTagUsage(MemoryTagValue tag)
{
  assert(tag < v_max_tags_);
  return v_tag_usage_[tag].load();
}

const char* gdm::MemoryTracker::GetTagName(MemoryTagValue tag)
{
  assert(tag < v_max_tags_);
  return v_tag_names_[tag];
}
