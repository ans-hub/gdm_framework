// *************************************************************
// File:    memory_tag.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MEM_TAG_H
#define AH_GDM_MEM_TAG_H

#include "system/hash_utils.h"

#define MEMORY_TAG(name) ::gdm::MemoryTag<GDM_HASH(name)>{name}

namespace gdm {

#ifndef NDEBUG
  template <size_t Value>
  struct MemoryTag
  {
    operator int() const;
    const char* name_;

}; // struct MemoryTag
#else
  template <size_t Value>
  using MemoryTag = int;
#endif

using MemoryTagValue = int;

} // namespace gdm

#include "memory_tag.inl"

#endif // AH_GDM_MEM_TAG_H