// *************************************************************
// File:    rand_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "rand_utils.h"

// --private

static std::default_random_engine& global_urng( )
{
  static std::default_random_engine u{};
  return u;
}

// --public

inline void gdm::rand::StartRand()
{
  static std::random_device rd{};
  global_urng().seed( rd() );
}

inline int gdm::rand::GetRand(int from, int to)
{
  static std::uniform_int_distribution<> d{};
  using parm_t = decltype(d)::param_type;
  return d(global_urng(), parm_t{from, to});
}

inline float gdm::rand::GetRand(float from, float to)
{
  static std::uniform_real_distribution<> d{};
  using parm_t = decltype(d)::param_type;
  return static_cast<float>(d(global_urng(), parm_t{from, to}));
}

inline int gdm::rand::CoinToss()
{
  return GetRand(0,1);
}

inline int gdm::rand::GetSequenceCount(int from, int to, int limits)
{
  int i{0};
  int val = GetRand(from, to);
  while (true) {
    int next = GetRand(from, to);
    if (++i >= limits) return i;
    if (next != val) return i;
    val = next;
  }
}

inline int gdm::rand::GetCoinSequenceCount(int limits)
{
  return GetSequenceCount(0, 1, limits);
}
