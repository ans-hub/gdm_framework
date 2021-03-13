// *************************************************************
// File:    vector4.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_VECTOR4_H
#define AH_GDM_VECTOR4_H

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#endif

#include "math/vector3.h"

namespace gdm {

template<class T>
struct Vector4
{
  union {
    struct { T x, y, z, w; };
    struct { T r, g, b, a; };
    struct { T s, t, p, l; };
    struct { T l, r, t, b; };
    struct { T x, y, u, v; };
    struct { T x0, y0, x1, y1; };
    struct { T u0, v0, u1, v1; };
    struct { T data[4]; };
  };

  constexpr Vector4() = default;
  constexpr explicit Vector4(T val) : x{val}, y{val}, z{val}, w{val} { }
  constexpr explicit Vector4(T arr[4]) : x{arr[0]}, y{arr[1]}, z{arr[2]}, w{arr[3]} { }
  constexpr Vector4(T ax, T ay, T az, T aw) : x{ax}, y{ay}, z{az}, w{aw} { }
  constexpr Vector4(const Vector3<T>& v, float aw) : x{v.x}, y{v.y}, z{v.z}, w{aw} { }
  constexpr Vector4(const Vector4<T>& v, float aw) : x{v.x}, y{v.y}, z{v.z}, w{aw} { }
  constexpr Vector4(T val, T fourth_comp) : x{val}, y{val}, z{val}, w{fourth_comp} { }
  
  Vector3<T> xyz() const { return Vector3<T>(x,y,z); }
  auto raw() const -> decltype(auto) { return data[4]; }

  T operator[](int i) const { return (i == 0 ? x : (i == 1 ? y : (i == 2 ? z : (i == 3 ? w : w)))); }
  T& operator[](int i) { return (i == 0 ? x : (i == 1 ? y : (i == 2 ? z : (i == 3 ? w : w)))); }

}; // struct Vector4

template<class T>
using Vec4 = Vector4<T>;

using Vec4f = Vec4<float>;
using Vec4u = Vec4<unsigned int>;
using Vec4i = Vec4<int>;

} // namespace gdm

#endif  // AH_GDM_VECTOR4_H
