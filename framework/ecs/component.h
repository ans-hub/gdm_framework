// *************************************************************
// File:    component.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_ECS_COMP_H
#define AH_ECS_COMP_H

#include <utility>
#include <cassert>

#include "core.h"
#include "helpers.h"

namespace ecs {

template<std::size_t N>
struct Component
{
  constexpr static unsigned Sig() { return sig; }
  static void SetStorage(void* storage_in, unsigned stride_in);
  template<class T> static T* GetStorage(EntityId eid);
  template<class T> static void InitializeForEntity(T& self, T&& value);
  const static unsigned sig = N;
  static void* storage;
  static unsigned stride;
};

template<std::size_t N>
struct SingletonComponent : Component<N>
{
  SingletonComponent();
  template<class T> static T* GetStorage(EntityId /* eid */);
  template<class T> static void InitializeForEntity(T& self, T* value);
};

/* COMPONENTS DEFINITION */

template<std::size_t N>
void* Component<N>::storage = nullptr;

template<std::size_t N>
unsigned Component<N>::stride = 0;

template<std::size_t N>
inline void Component<N>::SetStorage(void* storage_in, unsigned stride_in)
{
  assert(storage == nullptr && "Try to register component not in header (either main or cpp");
  storage = storage_in;
  stride = stride_in;
}

template<std::size_t N>
template<class T>
inline T* Component<N>::GetStorage(EntityId eid)
{
  assert(storage && "Storage of component is empty, possible unregistered");
  assert(eid < k_max_eid);
  return (T*)((char*)(storage)+(stride*eid));
}

template<std::size_t N>
template<class T>
inline void Component<N>::InitializeForEntity(T& self, T&& value)
{
  std::swap(self, value);
}

template<std::size_t N>
SingletonComponent<N>::SingletonComponent()
{
  static int i = 0;
  assert(i == 0 && "Singleton component shouldn't be initiialized twice, use pointer type in passing CreateEntity");
  i++;
}

template<std::size_t N>
template<class T>
inline T* SingletonComponent<N>::GetStorage(EntityId /* eid */)
{
  return (T*)SingletonComponent<N>::storage;
}

template<std::size_t N>
template<class T>
inline void SingletonComponent<N>::InitializeForEntity(T& /*self*/, T* /*value*/) { }

// Helper, need to call SetStorage fn outside main()
// We need helper SetStorage as we can't set directly to static members through derived
// Also we need to register component or in cpp file or in main as any include of of file
//  from different obj causes redefinition and therefore different memory addresses

template<class T>
struct SetComponentStorageAtCompileTime {
  SetComponentStorageAtCompileTime(void* p, unsigned s)
  {
    assert(T::storage == nullptr && "Try to declare component in cpp or in main");
    T::SetStorage(p,s);
  }
};

} // namespace ecs

/* MACROSES */

#define ECS_COMPONENT_IDX 1 << (__COUNTER__ + ecs::ReservedSigs::k_count)

#define ECS_COMPONENT_REGISTER(name)\
inline static ecs::helpers::RedefinitionAssert<name,name> s_comp_##name##checker{};\
inline static name s_comp_##name[ecs::k_max_eid];\
inline static ecs::SetComponentStorageAtCompileTime<name> s_aux_##name(&s_comp_##name, sizeof(name));\
static_assert(std::is_default_constructible<name>::value, "Component should be default constructible");\

#define ECS_COMPONENT_SINGLETON_REGISTER(name, ...)\
inline static ecs::helpers::RedefinitionAssert<name,name> s_comp_##name##checker{};\
inline static name s_comp_single_##name {__VA_ARGS__};\
inline static ecs::SetComponentStorageAtCompileTime<name> s_aux_##name(&s_comp_single_##name, sizeof(name));\

#define ECS_DEFINE_ACCESS_OPERATORS(var)\
  auto* operator->() { return &var; }\
  const auto* operator->() const { return &var; }\
  auto& operator*() { return var; }\
  const auto& operator*() const { return var; }\

// Note: register singleton components in particular order in one places  

/*
   SPECIAL COMPONENT TYPES - performs add to systems eid and dt, as these
   components are applied implicit to entities signatures
*/

namespace ecs {

struct Dt : SingletonComponent<ReservedSigs::DT>
{
  float Get() const { return dt; }
  float& operator=(float val) { dt = val; return dt; }
  float operator*() const { return dt; }
  float dt;
};
ECS_COMPONENT_SINGLETON_REGISTER(Dt)

struct Eid : Component<ReservedSigs::EID>
{
  Eid(){
    static EntityId i = 0;
    eid = i;
    ++i;
  }
  EntityId Get() const { return eid; }
  EntityId operator*() const { return eid; }
  EntityId eid;
};
ECS_COMPONENT_REGISTER(Eid)

} // namespace ecs

#endif // AH_ECS_COMP_H