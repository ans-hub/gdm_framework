// *************************************************************
// File:    system.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "system.h"

// --public

template<class...Args>
inline ecs::System::System(const char* name, unsigned hash, std::tuple<Args...>&& tuple)
  : entities_{}
  , sig_{}
  , name_{name}
  , hash_{hash}
{
  RegisterSystem();
  ComputeSystemSignature(std::move(tuple));
}

template<class...Args>
inline void ecs::System::ComputeSystemSignature(std::tuple<Args...>&& tuple)
{
  helpers::ForeachTuple(tuple, [this](const auto* t){ sig_ |= t->Sig(); });
}

template<class T>
ecs::helpers::EcsRequireAdd<T>::EcsRequireAdd(unsigned shash, std::size_t element_offset)
{
  std::size_t sz = sizeof(T);
  T* storage = T::template GetStorage<T>(0ull);
  void* ptr = (void*)((char*)(storage) + element_offset);
  s_systems_requires[shash].insert(std::make_pair(sz, ptr));
}
