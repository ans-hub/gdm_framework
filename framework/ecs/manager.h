// *************************************************************
// File:    manager.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_MGR_H
#define AH_ECS_MGR_H

#include <array>
#include <algorithm>
#include <numeric>
#include <queue>
#include <vector>
#include <utility>
#include <functional>
#include <unordered_map>
#include <cassert>
#include <type_traits>
#include <tuple>

#include "core.h"
#include "system.h"
#include "component.h"
#include "event.h"
#include "helpers.h"

#ifdef _MSC_VER
// set /experimental:preprocessor /Wv:18 for correct macro expanding
#endif

namespace ecs {

struct EntityManager
{
  EntityManager();

public:
  template <class...Args>
  auto CreateEntity(std::tuple<Args...>&& t = {}) -> EntityId;
  template <class T>
  auto GetComponent(EntityId eid = 0) -> T& { return *(T::template GetStorage<T>(eid)); }
  template <class T, class... Args>
  void SendEventBroadcast(Args &&... args);
  template <class...Args>
  void AddComponentsToEntity(EntityId eid, std::tuple<Args...>&& t = {});
  template <class...Args>
  void RemoveComponentsFromEntity(EntityId eid, std::tuple<Args...>&& t = {});

public:
  auto GetFreeEid() -> EntityId;
  void PushFreeEid(EntityId eid);
  void DeleteEntity(EntityId eid);
  void DeleteEntities(const std::vector<EntityId>& eids);
  void Tick(float dt);

public:
  static auto GetInstance() -> EntityManager&;

private:
  void DeleteEntities();
  void ExecuteSystems();
  void ClearEvents();
  void RegisterEntityInSystems(EntityId eid, unsigned esig);
  void UnregisterEntityFromSystems(EntityId eid, unsigned old_esig, unsigned del_esig);

  std::vector<EntityId> s_eid_pool_;
  std::vector<EntityId> s_eid_create_queue_;
  std::vector<EntityId> s_eid_delete_queue_;
  std::unordered_map<unsigned, std::vector<int>> s_map_esig_to_systems_;
  std::unordered_map<unsigned, std::vector<int>> s_map_esig_to_events_;
  std::vector<unsigned> s_map_eid_to_esig_;

}; // struct EntityManager

namespace debug
{
  std::vector<EntityId> GetAliveEntities();
  std::vector<std::string> GetTickSystemExecLog();

} // namespace debug

} // namespace ecs

#include "manager.inl"

#endif // AH_ECS_MGR_H