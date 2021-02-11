// *************************************************************
// File:    type_traits.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TYPE_TRAITS_H
#define AH_GDM_TYPE_TRAITS_H

#include <string>

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

  struct NoneType {};

  template <template<class...> class Op, class... Args>
  constexpr inline static bool v_IsDetected = _private::Detector<NoneType, void, Op, Args...>::DETECTED;

	template<typename T>
	struct TypeId
	{
    operator int() { return index_; }
    operator int() const { return index_; }

	private:
		static int index_;
	};

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

#include "type_traits.inl"

#endif // AH_GDM_TYPE_TRAITS_H