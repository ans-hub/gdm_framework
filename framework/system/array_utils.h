// *************************************************************
// File:    array_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_ARRAY_H
#define AH_GDM_ARRAY_H

#include <vector>

namespace gdm::arr_utils {

  // Normalizes data in array by given [min;max]

  template<class T>
  void Normalize(T* data, std::size_t size, const T& min, const T& max);

  // Expands container by data (i.e. given 2 arrays with vec3 and vec4, then the
  // result will be array with vec3vec4,...,vec3vec4)

  template<class T>
  std::vector<T> Expand(const T* v1, std::size_t v1_val_sz, std::size_t v1_sz,
                        const T* v2, std::size_t v2_val_sz, std::size_t v2_sz);
  
  // Create a vector<T> from T array

  template<class T>
  std::vector<T> ArrayToVector(const T* v, std::size_t v_sz, std::size_t type_sz);

  template<class T, class K>
  void EnsureIndex(std::vector<T>& v, K index);

} // namespace gdm::arr_utils

#include "array_utils.inl"

#endif  // AH_GDM_ARRAY_H