// *************************************************************
// File:    general.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_
// *************************************************************

#ifndef AH_GDM_GENERAL_H
#define AH_GDM_GENERAL_H

#include <cmath>
#include <limits>
#include <memory>
#include <utility>

namespace gdm::math {

  constexpr float kEpsilon = std::numeric_limits<float>::epsilon();
  constexpr float kPI = 3.1415926535f;
  constexpr float kPI_mul2 = 6.283185307f;
  constexpr float kPI_div2 = 1.570796327f;
  constexpr float kPI_div4 = 0.785398163f;
  constexpr float kPI_inv = 0.318309886f;

  template<class T>
  constexpr T Abs(T n) { return n < 0 ? -n : n; }
  constexpr int Floor(float n) { return (int)(n); }
  constexpr int Ceil(float n) { return (int)(n + 1.0f); }
  constexpr bool FZero(float n) { return math::Abs(n) < kEpsilon; }
  constexpr bool FNotZero(float n) { return !(math::Abs(n) < kEpsilon); }
  constexpr bool FLessZero(float n) { return n < -kEpsilon; }
  constexpr bool FEq(float n1, float n2) { return math::Abs(n1-n2) < kEpsilon; }
  constexpr bool IsNan(float v) { return v == v; }
  constexpr bool IsAbsFactorOfTwo(int);
  constexpr int Clamp(int, int, int);
  float Clamp(float, float, float);
  template<class I>
  constexpr bool IsPowerOfTwo(I num) { return num > 0 && (num & (num - 1)) == 0; }
  template<class T = float>
  auto LeadToRange(const std::pair<T,T>& src, const std::pair<T,T>& dst, T val) -> float;
  template<class T>
  auto Min(const T& a, const T& b) -> T { return (a < b) ? a : b; }
  template<class T>
  auto Max(const T& a, const T& b) -> T { return (a > b) ? a : b; }
  template<class T>
  void Swap(T& a, T& b) { T t {std::move(a)}; a = std::move(b); b = std::move(t); }

} // namespace gdm::math

namespace gdm::trig {

  constexpr float Deg2rad(float deg) { return deg * 3.1415926535f / 180.0f; }
  constexpr float Rad2deg(float rad) { return rad * 180.0f / 3.1415926535f; }
  constexpr float Deg2rad(int deg) { return Deg2rad(static_cast<float>(deg)); }
  constexpr float Rad2deg(int rad) { return Rad2deg(static_cast<float>(rad)); }
  float CalcOppositeCatet(float adjanced_catet, float theta);

} // namespace gdm::trig

template <>
inline double gdm::math::Abs<double>(const double x)
{
  uint64_t i = reinterpret_cast<const std::uint64_t&>(x);
  i &= 0x7FFFFFFFFFFFFFFFULL;
  return reinterpret_cast<const double&>(i);
}

inline constexpr bool gdm::math::IsAbsFactorOfTwo(int num)
{
  int n = math::Abs(num);
  return !(n & (n-1));
}

inline constexpr int gdm::math::Clamp(int val, int min, int max)
{
  int rem = val % max;
  return rem < 0 ? max + rem : min + rem;
}

inline float gdm::math::Clamp(float val, float min, float max)
{
  float rem = std::fmod(val, max);
  return rem < 0.0f ? max + rem : min + rem;
}

// Make value from range 1 as value from range 2 

template<class T>
inline float gdm::math::LeadToRange(const std::pair<T,T>& range_src, const std::pair<T,T>& range_dest, T val)
{
  auto min_src = range_src.first;
  auto max_src = range_src.second;
  auto min_dest = range_dest.first;
  auto max_dest = range_dest.second;

  auto val_pos    = val / (max_src - min_src);
  auto val_offset = val_pos * (max_dest - min_dest);
  
  return min_dest + val_offset;
}

// Computes opposite catet in triangle with right angle:
//  cos_& = adj / hyp
//  hyp   = adj / cos_&
//  opp   = sin_& * hyp
// Also we may compute oppisite catet from tg:
//  tg_& = sin_& / cos_&
//  tg_& = opp / adj
//  opp = tg_& * adj = (sin_& / cos_&) * adj 

inline float gdm::trig::CalcOppositeCatet(float adjanced_catet, float theta)
{
  float cosine = std::cos(theta);
  float hypotenuse = adjanced_catet / cosine;
  float opposite_catet = std::sin(theta) * hypotenuse;
  return opposite_catet;
}

#endif  // AH_GDM_GENERAL_H