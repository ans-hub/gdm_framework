// *************************************************************
// File:    mt_utils.inl
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "mt_utils.h"

#include <utility>

//--private

namespace gdm::_private {

struct Notificator
{

}; // struct Notificator

} // namespace gdm::_private

//--public

template <class Fn, class K>
void gdm::mt::WhenAll(Fn&& fn, const std::vector<std::future<K>>& futures)
{

}