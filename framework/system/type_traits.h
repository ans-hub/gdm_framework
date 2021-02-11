// *************************************************************
// File:    type_traits.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TYPE_TRAITS_H
#define AH_GDM_TYPE_TRAITS_H

namespace gdm {

  struct NoneType {};

  template <template<class...> class Op, class... Args>
  constexpr inline static bool v_IsDetected = _private::Detector<NoneType, void, Op, Args...>::DETECTED;

} // namespace gdm

#include "type_traits.inl"

#endif // AH_GDM_TYPE_TRAITS_H