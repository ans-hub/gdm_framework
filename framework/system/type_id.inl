// *************************************************************
// File:    type_id.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "type_id.h"

//--public

namespace gdm::_private {

  template <class T>
  int GetCounterValue()
  {
    static int counter = v_static_time_counter++;
    return counter;
  }
	
	static inline int v_static_time_counter = 0;

} // namespace gdm::_private


template<typename T>
int gdm::TypeId<T>::index_ = _private::GetCounterValue<T>();
