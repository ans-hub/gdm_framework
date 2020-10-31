// *************************************************************
// File:    event.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// ecs::Event is a simple component that is implicitly added to any entity
// (with sig 3). Template of ecs::Event is component too but have 0 sig.
// It works like all other implicit components. When event system is registered
// we add there pointer to storage of ecs::Event<T>. That is all.

#ifndef AH_ECS_EVENT_H
#define AH_ECS_EVENT_H

#include <queue>

#include "helpers.h"
#include "core.h"
#include "component.h"
#include "system.h"

namespace ecs {

/* EVENT DECLARATION */

struct EventBase
{
  EventBase();
  virtual void Clear() =0;
  void RegisterEvent();
};

template<class M>
struct Event : EventBase
{
  Event();
  constexpr static unsigned Sig() { return ReservedSigs::EVENT; }
  static void SetStorage(void* storage_in, unsigned stride_in);
  template<class T> static T* GetStorage(EntityId /* eid */); // todo: Event<M> static Event<M>
  template<class T> static void InitializeForEntity(T& self, T* value) { }
  virtual void Clear() override { s_data.clear(); s_data.resize(0); }
  static std::vector<M> s_data;
  static void* storage;
  static unsigned stride;
};

/* EVENT DEFINITION */

inline static std::array<ecs::EventBase*, k_max_components> s_events;

template<class M>
std::vector<M> Event<M>::s_data = {};

template<class M>
void* Event<M>::storage = nullptr;

template<class M>
unsigned Event<M>::stride = 0;

template<class M>
inline Event<M>::Event()
  : EventBase{}
{ }

inline EventBase::EventBase()
{
  RegisterEvent();
}

inline void EventBase::RegisterEvent()
{
  static int s_event_idx = 0;
  s_events[s_event_idx] = this;
  ++s_event_idx;
}

template<class M>
inline void Event<M>::SetStorage(void* storage_in, unsigned stride_in)
{
  assert(storage == nullptr && "Redefinition of event");
  storage = storage_in;
  stride = stride_in;
}

template<class M>
template<class T>
inline T* Event<M>::GetStorage(EntityId eid)
{
  return !s_data.empty() ? (T*)Event<M>::storage : nullptr;
}

} // namespace ecs

/* EVENT MACRO */

#define ECS_COMPONENT_BROADCAST_EVENT_REGISTER(name)\
inline static ecs::helpers::RedefinitionAssert<name,name> s_event_##name##checker{};\
inline static ecs::Event<name> s_event_##name {};\
inline static ecs::SetComponentStorageAtCompileTime<ecs::Event<name>> s_aux_##name(&s_event_##name, sizeof(name));\

#define ECS_COMPONENT_ADDRESS_EVENT_REGISTER(name)\
/*inline static ecs::helpers::RedefinitionAssert<name,name> s_event_##name##checker{};*/\
inline static ecs::Event<name> s_event_##name;\
inline static ecs::SetComponentStorageAtCompileTime<ecs::Event<name>> s_event_aux_##name(&s_event_##name, sizeof(name));\

#define ECS_SYSTEM_EVENT_REGISTER(func, ...) ECS_SYSTEM_REGISTER(func, __VA_ARGS__)

/* BUILT-IN EVENTS */

namespace ecs {

struct OnCreateEntity : Component<0>
{
  bool created = false;
};
ECS_COMPONENT_REGISTER(OnCreateEntity)
ECS_COMPONENT_ADDRESS_EVENT_REGISTER(OnCreateEntity)

template<>
template<>
inline Event<OnCreateEntity>* Event<OnCreateEntity>::GetStorage(EntityId eid)
{
  return (Event<OnCreateEntity>*)Event<OnCreateEntity>::storage;
}

template<>
inline void Event<OnCreateEntity>::Clear()
{
  for (std::size_t eid = 0; eid < k_max_eid; ++eid)
    OnCreateEntity::GetStorage<OnCreateEntity>(eid)->created = false;
}

} // namespace ecs

#endif // AH_ECS_EVENT_H
