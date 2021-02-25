// *************************************************************
// File:    rtti.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************


#include "rtti.h"

#include <regex>

#include "type_traits.h"

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

//--private

namespace gdm::_private {

  template <class T>
  using HasBase = decltype((void)std::declval<T::TBase>);

  template <class T>
  using HasId = decltype(std::declval<T>().Id());

} // namespace gdm::_private

//--public

template <class Base>
template <class Derived>
inline bool gdm::IsClass<Base>::BaseTo()
{
  using B = std::decay_t<Base>;
  using D = std::decay_t<Derived>;

  static_assert(v_IsDetected<_private::HasId, B>, "Base is non rtti-friendly type");
  static_assert(v_IsDetected<_private::HasId, D>, "Derived is non rtti-friendly type");

  if constexpr (v_IsDetected<_private::HasBase, D>)
    return B::Id() == D::Id() || IsClass<B>::template BaseTo<D::TBase>();
  else
    return B::Id() == D::Id();
}

template<class T, T& var>
inline std::string gdm::GetInstanceName()
{
  std::string str(__PRETTY_FUNCTION__);
  std::smatch match;
  std::regex pattern("^\\*\\bGetInstanceName<int,(\\*)>\\(void\\)$");
  std::regex_search(str, match, pattern);

  return match.empty() ? str : match[1];
}
