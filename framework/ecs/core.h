// *************************************************************
// File:    core.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_CORE_H
#define AH_ECS_CORE_H

#include <array>
#include <utility>
#include <functional>

#include <system/hash_utils.h>

namespace ecs {

using EntityId = unsigned;

static constexpr int k_max_eid = 1024;
static constexpr int k_max_components = 32;
static constexpr int k_max_systems = 1024;

template<std::size_t N>
struct CompileTimeCounter
{
  constexpr static std::size_t val = N;
};

// Reserved sigs performs adding to each entity implicit components to
// perform on-the-fly adding of systems without adding components

enum ReservedSigs : unsigned
{
  DT = 1<<0,
  EID = 1<<1,
  EVENT = 1<<2,
  k_count = 3
};

constexpr static int GetReservedSigsMask() { return (1 << ReservedSigs::k_count) - 1; } 

} // namespace ecs

#endif // AH_ECS_CORE_H