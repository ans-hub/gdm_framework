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

} // namespace gdm::_private 
