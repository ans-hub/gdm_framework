// *************************************************************
// File:    type_traits.inl
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "type_traits.h"

//-- private

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

  template <class T>
  using HasBase = decltype((void)std::declval<T::TBase>);

  template <class T>
  using HasId = decltype(std::declval<T>().Id());

  static inline int v_static_time_counter = 0;

  template <class T>
  int GetCounterValue()
  {
    static int counter = v_static_time_counter++;
    return counter;
  }

} // namespace gdm::_private 

//-- public

template<typename T>
int gdm::TypeId<T>::index_ = _private::GetCounterValue<T>();

template <class Base>
template <class Derived>
inline bool gdm::IsClass<Base>::Of()
{
  using B = std::decay_t<Base>;
  using D = std::decay_t<Derived>;

  static_assert(v_IsDetected<_private::HasId, B>, "Base is non rtti-friendly type");
  static_assert(v_IsDetected<_private::HasId, D>, "Derived is non rtti-friendly type");

  if constexpr (v_IsDetected<_private::HasBase, D>)
    return B::Id() == D::Id() || IsClass<B>::Of<D::TBase>();
  else
    return B::Id() == D::Id();
}
