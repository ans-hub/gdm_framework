// *************************************************************
// File:    type_id.cc
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "type_id.h"

//--public

int& gdm::GetCounterGlobal()
{
  static int v_static_time_counter = 0;
  return v_static_time_counter;
}
