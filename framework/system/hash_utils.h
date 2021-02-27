// *************************************************************
// File:    hash_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_HASH_UTILS_H
#define AH_GDM_HASH_UTILS_H

#pragma warning(disable: 4244)

#include <stdint.h>

#define GDM_HASH(x) ::gdm::hash::Djb2CompileTime(x)
#define GDM_HASH_N(x,n) { ::gdm::hash::CombinedHashSimple<GDM_HASH(x)>(n) }
#define GDM_HASH_S(x,s) { ::gdm::hash::CombinedHashFull<GDM_HASH(x)>(GDM_HASH(s)) }

namespace gdm {
  using Hash = uint64_t;
}

namespace gdm::hash {

  uint64_t Djb2(const char* str);
  static constexpr uint64_t Djb2CompileTime(const char* str, uint64_t hash = 5381);
  template<size_t idx>
  static constexpr uint32_t Crc32(const char* str);
  static constexpr uint64_t CombineHash(const uint64_t& lhs, const uint64_t& rhs);
  template<uint64_t Part1>
  constexpr uint64_t CombinedHashFull(uint64_t part2);
  template<uint64_t Part1>
  constexpr uint64_t CombinedHashSimple(uint64_t part2);


} // namespace gdm::hash

#include "hash_utils.inl"

#endif  // AH_GDM_HASH_UTILS_H
