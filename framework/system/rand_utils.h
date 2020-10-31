// *************************************************************
// File:    rand_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_RAND_H
#define AH_GDM_RAND_H

#include <random>

namespace gdm::rand {

  void StartRand();
  int GetRand(int from, int to);
  float GetRand(float from, float to);
  int CoinToss();
  int GetSequenceCount(int from, int to, int limits);
  int GetCoinSequenceCount(int limits);

} // namespace gdm::rand

#include "rand_utils.inl"

#endif  // AH_GDM_RAND_H