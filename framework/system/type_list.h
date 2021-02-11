// *************************************************************
// File:    type_list.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_TYPE_LIST_H
#define AH_GDM_TYPE_LIST_H

namespace gdm {

  template <class...T>
  struct TypeList
  { };
	
  template <typename... T>
	constexpr auto v_TypeList = TypeList<T...>{};

} // namespace gdm

#include "type_list.inl"

#endif // AH_GDM_TYPE_LIST_H