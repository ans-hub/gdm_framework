// *************************************************************
// File:    system.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#pragma warning(disable:4267)

#ifndef AH_ECS_SYS_H
#define AH_ECS_SYS_H

#include <array>
#include <utility>
#include <functional>
#include <unordered_map>
#include <set>
#include <cassert>

#include "helpers.h"
#include "core.h"

namespace ecs {

struct System
{
  template<class...Args>
  System(const char* name, unsigned hash, std::tuple<Args...>&& tuple);
  void RegisterSystem();
  template<class...Args>
  void ComputeSystemSignature(std::tuple<Args...>&& tuple);
  virtual void Call(EntityId eid) =0;
  virtual bool CheckSingletonRequires(EntityId eid = 0) =0;
  bool CheckEntityRequires(EntityId eid);
  unsigned Sig() const { return sig_; }

  std::array<EntityId, k_max_eid> entities_;
  unsigned sig_;
  const char* name_;
  unsigned hash_;
};

namespace helpers
{
  template<class T>
  struct EcsRequireAdd
  {
    EcsRequireAdd(unsigned shash, std::size_t offset);
  };
  System* GetSystem(unsigned shash);

} // namespace helpers

// System holders

inline static std::array<System*, k_max_systems> s_systems {};
inline static std::unordered_map<unsigned, int> s_sysname_to_system {57};
inline static std::unordered_map<unsigned, std::set<std::pair<std::size_t, void*>>> s_systems_requires {57};

} // namespace ecs

// Private helper macroses to build string tokens

#define _ECS_CONCAT(a,b) a##b
#define _ECS_XSTR(s) #s
#define _ECS_STR(s) _ECS_XSTR(s)
#define _ECS_DEREF(a) *(a)

// Private helper macroses to main System macros

#define _ECS_SYSVAR(a,b) s_sys_##a##b

#define _ECS_CHECKVAR(a,b) s_sys_check_##a##b

#define _ECS_TYPE_TO_PTR0(a) a*
#define _ECS_TYPE_TO_PTR(...) FOR_EACH_LIST(_ECS_TYPE_TO_PTR0, __VA_ARGS__)

#define _ECS_GET_STORAGE0(a) _ECS_DEREF(a::GetStorage<a>(eid))
#define _ECS_GET_STORAGE(...) FOR_EACH_LIST(_ECS_GET_STORAGE0,__VA_ARGS__)

#define _ECS_GET_STORAGE_PTR0(a) a::GetStorage<a>(eid)
#define _ECS_GET_STORAGE_PTR(...) FOR_EACH_AND(_ECS_GET_STORAGE_PTR0, __VA_ARGS__)

// Public helper macroses to build System class

// 1. we create static system variable to call system base ctor, register it
//    is global system variable and be alive all time
// 2. we use pointer as we won't to call constructions on tuple iterationg
//    this is matter on singleton componenets

#define ECS_SYSTEM_REGISTER(func, ...)\
template<class...Args>\
struct _ECS_CONCAT(func,System) : ecs::System\
{\
  _ECS_CONCAT(func,System)() : System(_ECS_STR(func), ECS_HASH(#func), std::tuple<Args...>{}){ }\
  virtual void Call(ecs::EntityId eid) override { func(_ECS_GET_STORAGE(__VA_ARGS__)); }\
  virtual bool CheckSingletonRequires(ecs::EntityId eid = 0) override { return _ECS_GET_STORAGE_PTR(__VA_ARGS__); }\
};\
inline _ECS_CONCAT(func,System)<_ECS_TYPE_TO_PTR(__VA_ARGS__)> _ECS_SYSVAR(func,System);\

#define ECS_REQUIRE(func, type, var)\
inline static ecs::helpers::EcsRequireAdd<type> _ECS_CONCAT(var,func) (ECS_HASH(#func), offsetof(type,var));

#include "system.inl"

#endif // AH_ECS_SYS_H