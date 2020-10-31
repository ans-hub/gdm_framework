// *************************************************************
// File:    array_utils.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "array_utils.h"

#include <cassert>

template<class T>
inline void gdm::arr_utils::Normalize(T* data, std::size_t size, const T& min, const T& max)
{
  T tmax {};
  T tmin {};
  const T* end = &data[size];
  for (T* curr = data; curr != end; ++curr)
  {
    tmax = *curr > tmax ? *curr : tmax;
    tmin = *curr < tmin ? *curr : tmin;
  }
  float old_dt = tmax - tmin;
  float aim_dt = max - min;
  for (T* curr = data; curr != end; ++curr)
  {
    float mul = (*curr - tmin) / old_dt;
    *curr = min + (mul * aim_dt);
  }
}

template<class T>
inline std::vector<T> gdm::arr_utils::Expand(const T* v1, std::size_t v1_val_sz, std::size_t v1_sz,
                                             const T* v2, std::size_t v2_val_sz, std::size_t v2_sz)
{
  assert(v1 != nullptr);
  assert(v2 != nullptr);
  assert(v1_sz != 0 && v2_sz != 0);
  assert(v1_val_sz != 0 && v2_val_sz != 0);
  assert(v1_sz == v2_sz);

  std::vector<T> res {};
  res.reserve(v1_sz * v1_val_sz + v2_sz * v2_val_sz);

  for (std::size_t i = 0; i < v1_sz; ++i)
  {
    for (std::size_t k = 0; k < v1_val_sz; ++k)
      res.push_back(v1[i*v1_val_sz+k]);
    for (std::size_t k = 0; k < v2_val_sz; ++k)
      res.push_back(v2[i*v2_val_sz+k]);
  }
  assert(res.size() == v1_sz * v1_val_sz + v2_sz * v2_val_sz);
  return res;
}

template<class T>
inline std::vector<T> gdm::arr_utils::ArrayToVector(const T* v, std::size_t v_sz, std::size_t type_sz)
{
  assert(v);

  std::vector<T> res {};
  std::size_t new_sz = v_sz * type_sz;
  res.resize(new_sz);

  for (std::size_t i = 0; i < new_sz; ++i)
    res[i] = v[i];
  return res;
}

template<class T, class K>
inline void gdm::arr_utils::EnsureIndex(std::vector<T>& v, K index)
{
  assert(std::is_fundamental_v<K>);

  if (index >= v.size())
    v.resize(index + 1);  
}
