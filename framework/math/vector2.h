// *************************************************************
// File:    vector2.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <cmath>
#include <limits>

#ifndef AH_GDM_VECTOR2_H
#define AH_GDM_VECTOR2_H

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#endif

namespace gdm {

template<class T>
struct Vector2
{
  union {
    struct { T x, y; };
    struct { T r, g; };
    struct { T s, t; };
  };

  constexpr Vector2() = default;
  constexpr explicit Vector2(T val) : x{val}, y{val} { }
  constexpr Vector2(T ax, T ay) : x{ax}, y{ay} { }

  float Length() const { return std::sqrt(x*x, y*y); };
  float SqLength() const { return x*x + y*y; };
  void  Normalize();

  Vector2& operator*=(float scalar)
  {
    this->x *= scalar;
    this->y *= scalar;
    return *this;
  }
  Vector2& operator/=(float scalar)
  {
    this->x /= scalar;
    this->y /= scalar;
    return *this;
  }
  Vector2& operator+=(const Vector2& rhs)
  {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
  }
  Vector2& operator-=(const Vector2& rhs)
  {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this;
  }
  bool operator==(const Vector2& v)
  {
    return (std::abs(x-v.x) < std::numeric_limits<float>::epsilon()) &&
           (std::abs(y-v.y) < std::numeric_limits<float>::epsilon());
  }
  bool operator!=(const Vector2& v)
  {
    return !(*this == v);
  }
  friend inline Vector2 operator*(Vector2 lhs, float scalar)
  {
    lhs *= scalar;
    return lhs;
  }
  friend inline Vector2 operator/(Vector2 lhs, float scalar)
  {
    lhs /= scalar;
    return lhs;
  }
  friend inline Vector2 operator-(Vector2 lhs, const Vector2& rhs) {
    lhs -= rhs;
    return lhs;
  }
  friend inline Vector2 operator+(Vector2 lhs, const Vector2& rhs)
  {
    lhs += rhs;
    return lhs;
  }
  friend inline bool operator<(const Vector2& lhs, const Vector2& rhs)
  {
    return lhs.SqLength() < rhs.SqLength();
  }
  friend inline float operator*(const Vector2& v1, const Vector2& v2) // dot
  {
    return (v1.x * v2.x) + (v1.y * v2.y);
  }

}; // struct Vector2

template<class T>
using Vec2 = Vector2<T>;

using Vec2f = Vector2<float>;
using Vec2u = Vector2<unsigned int>;
using Vec2i = Vector2<int>;

namespace vec2 {

  inline Vec2f Flip90CW(const Vec2f& v) { return Vec2f(v.y, -v.x); }
  inline Vec2f Flip90CCW(const Vec2f& v) { return Vec2f(-v.y, v.x); }
  inline Vec2f GetPerpendicular(const Vec2f& v) { return Vec2f(v.y, -v.x); } 
  
  // Returns the length of imaginable 3d vec (or area of triangle by 2 vectors) by det of matrix
  inline float CrossProduct(const Vec2f& v1, const Vec2f& v2) { return (v1.x*v2.y)-(v1.y*v2.x); }

} // namespace vec2

template<class T>
inline void Vector2<T>::Normalize()
{
  if (std::abs(1.f - SqLength()) < std::numeric_limits<float>::epsilon())
    return;
  float len = Length();
  float p = static_cast<T>(1.f) / len;
  x *= p;
  y *= p;
}

} // namespace gdm

#endif  // AH_GDM_VECTOR2_H
