// *************************************************************
// File:    bits_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_BITS_UTILS_H
#define AH_GDM_BITS_UTILS_H

namespace gdm::bits {

  bool IsLittleEndian();
  auto Flip(const unsigned short& v) -> unsigned short;
  auto Flip(const unsigned int& v) -> unsigned int;

  template<class T>
  bool HasFlag(unsigned int flags, T flag);

} // namespace gdm::bits

#include "bits_utils.inl"

#endif  // AH_GDM_BITS_UTILS_H