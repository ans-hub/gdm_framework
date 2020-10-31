// *************************************************************
// File:    system.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "system.h"

// --public

void ecs::System::RegisterSystem()
{
  static int s_system_idx = 0;
  s_systems[s_system_idx] = this;
  s_sysname_to_system[ECS_HASH(name_)] = s_system_idx;
  ++s_system_idx;
}

bool ecs::System::CheckEntityRequires(ecs::EntityId eid)
{
  bool res = true;
  for(const auto& [sz, ptr] : s_systems_requires[hash_])
    res &= *(reinterpret_cast<bool*>(reinterpret_cast<char*>(ptr) + sz * eid));
  return res;
}

ecs::System* ecs::helpers::GetSystem(unsigned hname)
{
  auto found = s_sysname_to_system.find(hname);
  assert(found != s_sysname_to_system.end());
  assert(found->second < s_systems.size());
  return s_systems[found->second];
}
