// *************************************************************
// File:    manager.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "manager.h"

// --public 

// Pass a pointer for singleton components. As we work with tuples, we don't want to
//  unnecessary creation of heavy weighted objects, but still able to access static
//  member functions from nullptr
// Pass a rvalue for components that want to avoid copy (or copy is deleted)

template <class... Args>
inline void ecs::EntityManager::AddComponentsToEntity(EntityId eid, std::tuple<Args...>&& t)
{
  unsigned entity_sig = 0 | GetReservedSigsMask();

  // todo: need to be investigated why eid gets corrupted when passing by reference
 
  static auto init_component = [this,eid](auto &&elem) {
    auto &comp = GetComponent<typename std::remove_pointer<typename std::remove_reference<decltype(elem)>::type>::type>(eid);
    comp.InitializeForEntity(comp, std::move(elem));
  };
  static auto get_signature = [&](auto &&elem) {
    entity_sig |= std::remove_pointer<typename std::remove_reference<decltype(elem)>::type>::type::Sig();
  };

  helpers::ForeachTuple(t, init_component);
  helpers::ForeachTuple(t, get_signature);

  RegisterEntityInSystems(eid, entity_sig);
}

template <class... Args>
inline void ecs::EntityManager::RemoveComponentsFromEntity(EntityId eid, std::tuple<Args...>&& t)
{
  unsigned old_esig = s_map_eid_to_esig_[eid];
  unsigned del_esig = 0;

  static auto get_signature = [&](auto &&elem) {
    del_esig |= std::remove_pointer<typename std::remove_reference<decltype(elem)>::type>::type::Sig();
  };

  helpers::ForeachTuple(t, get_signature);
  if ((old_esig ^ GetReservedSigsMask()) == del_esig)
    DeleteEntity(eid);
  else
    UnregisterEntityFromSystems(eid, old_esig, del_esig);
}

template <class... Args>
inline ecs::EntityId ecs::EntityManager::CreateEntity(std::tuple<Args...>&& t)
{
  EntityId eid = GetFreeEid();
  assert(eid < k_max_eid);
  AddComponentsToEntity(eid, std::forward<std::tuple<Args...>>(t)); // See notes to AddComponentsToEntity
  OnCreateEntity::GetStorage<OnCreateEntity>(eid)->created = true;
  return eid;
}
