// *************************************************************
// File:    literals.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_LITERALS_H
#define AH_GDM_LITERALS_H

namespace gdm {

  using KiB = unsigned long long;
  using MiB = unsigned long long;
  using GiB = unsigned long long;

  constexpr unsigned long long operator""_Kb(KiB v) { return v * 1024; }
  constexpr unsigned long long operator""_Mb(MiB v) { return v * 1024 * 1024; }
  constexpr unsigned long long operator""_Gb(GiB v) { return v * 1024 * 1024 * 1024; }

} // namespace gdm

#endif  // AH_GDM_LITERALS_H