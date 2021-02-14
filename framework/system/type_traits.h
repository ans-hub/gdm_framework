// *************************************************************
// File:    type_traits.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TYPE_TRAITS_H
#define AH_GDM_TYPE_TRAITS_H

namespace gdm::_private
{
  template <class Default, class AlwaysVoid, template<class...> class Op, class... Args>
  struct Detector
  {
    constexpr static bool DETECTED = false;
    using Type = Default;
  };

  template <class Default, template<class...> class Op, class... Args>
  struct Detector<Default, std::void_t<Op<Args...>>, Op, Args...>
  {
    constexpr static bool DETECTED = true;
    using Type = Op<Args...>;
  };

} // namespace gdm::_private 

namespace gdm {

  struct NoneType {};

  template <template<class...> class Op, class... Args>
  constexpr inline static bool v_IsDetected = _private::Detector<NoneType, void, Op, Args...>::DETECTED;

} // namespace gdm

#include "type_traits.inl"

#endif // AH_GDM_TYPE_TRAITS_H