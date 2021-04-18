// *************************************************************
// File:    mt_utils.h
// Author:  Novoselov Anton @ 2021
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_MT_UTILS_H
#define AH_GDM_MT_UTILS_H

#include <future>
#include <vector>

namespace gdm::mt {

  template <class Fn, class K>
  void WhenAll(Fn&& fn, const std::vector<std::future<K>>& futures);

}  // namespace gdmLLnt

#include "mt_utils.inl"

#endif // AH_GDM_MT_UTILS_H
