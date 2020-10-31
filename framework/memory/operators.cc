// *************************************************************
// File:    operators.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "operators.h"

#include "memory/memory_manager.h"
#include "memory/memory_tag.h"

void* operator new(std::size_t size)
{
  return gdm::MemoryManager::Allocate(size);
}

void* operator new(std::size_t size, gdm::MemoryTagValue tag) noexcept
{
  return gdm::MemoryManager::Allocate(size, tag);
}

void operator delete(void* ptr, std::size_t size) noexcept
{
  return gdm::MemoryManager::Deallocate(ptr);
}

void operator delete(void* ptr, std::size_t size, gdm::MemoryTagValue tag) noexcept
{
  return gdm::MemoryManager::Deallocate(ptr, tag);
}
