// *************************************************************
// File:    memory_ut.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"
#include <vector>

#include <memory/aligned_allocator.h>
#include <memory/defines.h>
#include <memory/frame_allocator.h>
#include <memory/memory_manager.h>
#include <memory/helpers.h>
#include <memory/memory_tag.h>
#include <memory/memory_tracker.h>
#include <memory/operators.h>

#include <system/hash_utils.h>

using namespace gdm;

struct Empty { };

struct Small
{
  char bytes[32];
};

struct Big
{
  Big() = default;
  Big(int val) : value{val} {}
  char bytes[256];
  int value = 42;
};

__declspec(align(128)) struct Align128
{ };

TEST_CASE("Memory")
{
  SECTION("New/Delete")
  {
    {
      Empty* ptr = nullptr;
      ptr = GMNew Empty;
      CHECK(ptr != nullptr);

      uintptr_t uptr = mem::PtrToUptr(ptr);
      CHECK(uptr % 16 == 0);
      GMDelete(ptr);
    }
    {
      Big* ptr = nullptr;
      ptr = GMNew Big;
      CHECK(ptr != nullptr);
      
      uintptr_t uptr = mem::PtrToUptr(ptr);
      CHECK(uptr % 16 == 0);
      GMDelete(ptr);
    }
  }

  SECTION("Alloc/Dealloc")
  {
    void* ptr = MemoryManager::AllocateAligned(42, 32);
    CHECK(ptr != nullptr);
    CHECK(mem::IsAligned(ptr, 32));

    MemoryManager::DeallocateAligned(ptr, 32);
  }

  SECTION("Incorrect alignment")
  {
    CHECK_THROWS(MemoryManager::AllocateAligned(1, 33));
  }

  SECTION("Reallocation with another alignment")
  {
    void* ptr = MemoryManager::AllocateAligned(12, 16);
    ptr = MemoryManager::ReallocateAligned(ptr, 64, 32);
    CHECK(mem::IsAligned(ptr, 32));
    CHECK(MemoryManager::GetPointerSize(ptr) >= 64);
    CHECK(MemoryManager::GetPointerSize(ptr) < 64 + 32);
  }

  SECTION("Aligned struct")
  {
    void* ptr = nullptr;
    ptr = MemoryManager::AllocateAligned(1, alignof(Align128));
    CHECK(ptr != nullptr);
    CHECK(mem::IsAligned(ptr, 128));

    MemoryManager::DeallocateAligned(ptr, 128);
  }

  SECTION("Reallocate and placement new there")
  {
    void* ptr = nullptr;
    ptr = MemoryManager::AllocateAligned(sizeof(Small), alignof(Small));
    ptr = MemoryManager::ReallocateAligned(ptr, sizeof(Big), alignof(Big));

    new(ptr) Big {};
   
    Big* big = static_cast<Big*>(ptr);
    CHECK(big->value == 42);

    MemoryManager::DeallocateAligned(big, alignof(Big));
  }

  SECTION("Default allocator")
  {
    std::vector<int, AlignedAllocator<int, 32>> v;
    v.push_back({});
    v.push_back({});
    v.push_back({});
  }

  SECTION("Frame allocator - direct allocate")
  {
    const int big_count = 2;
    void* ptr_big = FrameAllocator<16384>::Allocate<Big>(big_count);
    CHECK(ptr_big != nullptr);
    CHECK(mem::IsAligned(ptr_big, 16));

    const int small_count = 2;
    void* ptr_small = FrameAllocator<16384>::Allocate<Small>(small_count, 32);
    CHECK(ptr_small != nullptr);
    CHECK(mem::IsAligned(ptr_small, 32));
    Big* big1 = GMNewPlaced(ptr_big) Big(42);
    ptr_big = (char*)ptr_big + sizeof(Big);
    Big* big2 = GMNewPlaced(ptr_big) Big{43};

    Small* small1 = GMNewPlaced(ptr_small) Small{};
    ptr_small = (char*)ptr_small + sizeof(Small);
    Small* small2 = GMNewPlaced(ptr_small) Small{};

    CHECK(big1->value == 42);
    CHECK(big2->value == 43);
    CHECK(small1);
    CHECK(small2);
  }

  SECTION("Frame allocator - container usage")
  {
    std::vector<int, FrameAllocatorTyped<int, 16384>> v;
    v.emplace_back();
    v.emplace_back();
    v.emplace_back();
    v.emplace_back();
  }

  SECTION("Tracked allocation")
  {
    void* p00 = MemoryManager::Allocate(sizeof(int) * 1, MEMORY_TAG("P0"));
    void* p01 = MemoryManager::Allocate(sizeof(int) * 2, MEMORY_TAG("P0"));
    void* p10 = MemoryManager::Allocate(sizeof(int) * 2, MEMORY_TAG("P1"));

    CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P0")) == sizeof(int) * 3);
    CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P1")) == sizeof(int) * 2);

    Small* s1 = GMNewTracked(MEMORY_TAG("P0")) Small;
    Small* s2 = GMNewTracked(MEMORY_TAG("P2")) Small;

    CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P0")) == sizeof(int) * 3 + sizeof(Small));
    CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P2")) == sizeof(Small));

    GMDeleteTracked(MEMORY_TAG("P0"), s1);
    CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P0")) == sizeof(int) * 3);

    {
      std::vector<int, AlignedAllocator<int>> v (AlignedAllocator<int>(MEMORY_TAG("P0")));
      v.push_back(0);
      v.push_back(1);
      v.push_back(2);
      CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P0")) == sizeof(int) * 6);
    }
    
    CHECK(MemoryManager::GetTagUsage(MEMORY_TAG("P0")) == sizeof(int) * 3);
  }
}
