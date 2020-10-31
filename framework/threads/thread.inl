// *************************************************************
// File:    thread.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************
 
// --public

#include "thread.h"

template <class Fx, class... Args>
inline gdm::Thread::Thread(Fx&& fx, Args&&... args)
  : is_running_{true}
  , thread_{std::move(fx), std::forward<Args>(args)...}
  , native_handle_{thread_.native_handle()}
{
  Init();
}
