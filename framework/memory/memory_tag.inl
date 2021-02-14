// *************************************************************
// File:    memory_tag.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "memory_tag.h"

#include <memory/memory_tracker.h>

#ifndef NDEBUG
template <size_t Value>
gdm::MemoryTag<Value>::operator int() const
{
  static const size_t s_reg_value_ = MemoryTracker::GetInstance().RegisterTag(name_);
  return static_cast<int>(s_reg_value_);
}
#endif
