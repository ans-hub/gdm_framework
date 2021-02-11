// *************************************************************
// File:    rtti.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_RTTI_H
#define AH_GDM_RTTI_H

#include <string>

#include <system/type_traits.h>
#include <system/type_id.h>

#define RTTI_BOTH(self)\
  template <class T_>\
  auto As() -> T_& { return static_cast<T_&>(*this); }\
  template <class T_>\
  auto As() const -> const T_& { return static_cast<const T_&>(*this); }\
	static int Id() { return gdm::TypeId<self>(); }\
  virtual auto Who() -> int { return Id(); }\

#define RTTI_ROOT(self)\
  template <class T_>\
  bool Is() const { return IsBaseId(T_::Id()); }\
  virtual bool IsBaseId(int base_id) const { return base_id == Id(); }\
  RTTI_BOTH(self)

#define RTTI_DECL(base, self)\
	using TBase = base;\
  virtual bool IsBaseId(int base_id) const { return base_id == Id() || TBase::IsBaseId(base_id); }\
  RTTI_BOTH(self)

namespace gdm {

  template <class Base>
  struct IsClass
  {
    using TType = std::decay_t<Base>;

    IsClass(const Base& instance) { }

    template <class Derived>
    static bool Of();

  }; // struct IsClass

  template<class T, T& var>
  std::string GetInstanceName();

} // namespace gdm

#include "rtti.inl"

#endif // AH_GDM_RTTI_H