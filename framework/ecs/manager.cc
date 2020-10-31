// *************************************************************
// File:    manager.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "manager.h"

// --public

ecs::EntityManager::EntityManager()
    : s_eid_pool_(ecs::k_max_eid)
    , s_eid_delete_queue_{}
    , s_map_esig_to_systems_{}
    , s_map_eid_to_esig_(ecs::k_max_eid)
{
  std::iota(s_eid_pool_.begin(), s_eid_pool_.end(), 0);
  std::make_heap(s_eid_pool_.begin(), s_eid_pool_.end(), std::greater<EntityId>());
}

ecs::EntityId ecs::EntityManager::GetFreeEid()
{
  assert(!s_eid_pool_.empty());
  std::pop_heap(s_eid_pool_.begin(), s_eid_pool_.end(), std::greater<EntityId>());
  EntityId eid = s_eid_pool_.back();
  s_eid_pool_.pop_back();
  return eid;
}

void ecs::EntityManager::PushFreeEid(EntityId eid)
{
  assert(s_eid_pool_.size() <= ecs::k_max_eid);
  s_eid_pool_.push_back(eid);
  std::push_heap(s_eid_pool_.begin(), s_eid_pool_.end(), std::greater<EntityId>());
}

void ecs::EntityManager::DeleteEntity(EntityId eid)
{
  s_eid_delete_queue_.push_back(eid);
  // *(OnDeleteEntity::GetStorage<OnDeleteEntity>(eid)) = true;
}

void ecs::EntityManager::DeleteEntities(const std::vector<EntityId>& eids)
{
  for(EntityId eid : eids)
    DeleteEntity(eid);
}

void ecs::EntityManager::Tick(float dt)
{
  *(Dt::GetStorage<Dt>(0)) = dt;
  DeleteEntities();
  ExecuteSystems();
  ClearEvents();
}

auto ecs::EntityManager::GetInstance() -> EntityManager&
{
  static EntityManager s_mgr;
  return s_mgr;
}

// --private

// todo: pass array of eids to register in systems

void ecs::EntityManager::RegisterEntityInSystems(EntityId eid, unsigned entity_sig)
{
  s_map_eid_to_esig_[eid] = entity_sig;
  auto systems_for_entity_sig = s_map_esig_to_systems_.find(entity_sig);
  if (systems_for_entity_sig == s_map_esig_to_systems_.end())
  {
    for (std::size_t i = 0; i < s_systems.size() && s_systems[i] != nullptr; ++i)
    {
      if ((s_systems[i]->Sig() & entity_sig) == s_systems[i]->Sig())
      {
        s_systems[i]->entities_[eid] = 1;
        s_map_esig_to_systems_[entity_sig].push_back(i);
      }
    }
  }
  else
  {
    for (unsigned system_index : systems_for_entity_sig->second)
      s_systems[system_index]->entities_[eid] = 1;
  }
}

void ecs::EntityManager::UnregisterEntityFromSystems(EntityId eid, unsigned old_esig, unsigned del_esig)
{
  unsigned new_esig = old_esig ^ del_esig;
  s_map_eid_to_esig_[eid] = new_esig;
  assert(s_map_esig_to_systems_.find(old_esig) != s_map_esig_to_systems_.end() && "Remove components from not created entity");
  
  int iter = 0;
  for (unsigned idx : s_map_esig_to_systems_[old_esig])
  {
    if ((s_systems[idx]->Sig() & new_esig) != s_systems[idx]->Sig())
      s_systems[idx]->entities_[eid] = 0;
    else
      s_map_esig_to_systems_[old_esig][iter++] = idx;
  }
  s_map_esig_to_systems_[old_esig].resize(iter);
}

void ecs::EntityManager::DeleteEntities()
{
  for (EntityId eid : s_eid_delete_queue_)
  {
    unsigned esig = s_map_eid_to_esig_[eid];
    for (int sidx : s_map_esig_to_systems_[esig])
      s_systems[sidx]->entities_[eid] = 0;
    s_map_eid_to_esig_[eid] = 0;
    PushFreeEid(eid);
  }
  s_eid_delete_queue_.clear();
}

void ecs::EntityManager::ExecuteSystems()
{
  for (std::size_t i = 0; i < s_systems.size() && s_systems[i] != nullptr; ++i)
  {
    System* s = s_systems[i];
    if (!s->CheckSingletonRequires())
      continue;
    for (std::size_t eid = 0; eid < s->entities_.size(); ++eid)
    {
      if (!s->CheckEntityRequires(eid))
        continue;
      if (s->entities_[eid] != 0)
        s_systems[i]->Call(eid);
    }
  }
}

// todo: get rid of s_events (think how to make it simple without s_events)

void ecs::EntityManager::ClearEvents()
{ 
  for (std::size_t i = 0; i < s_events.size() && s_events[i] != nullptr; ++i)
    s_events[i]->Clear();
}

template <class T, class... Args>
void ecs::EntityManager::SendEventBroadcast(Args &&... args)
{
  ecs::Event<T> &event = GetComponent<ecs::Event<T>>();
  event.s_data.push_back(std::move(T{std::forward<Args>(args)...}));
}
