// *************************************************************
// File:    vector3.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// Cross product vector for lhs need to be inverted as it is supposed
// that +z look from within the screen,

#include <cmath>
#include <cassert>
#include <array>
#include <limits>

#ifndef AH_GDM_VECTOR3_H
#define AH_GDM_VECTOR3_H

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#endif

namespace gdm {

template<class T>
struct Vector3
{
  union {
    struct { T x, y, z; };
    struct { T r, g, b; };
    struct { T s, t, p; };
    struct { T w, h, d; };
    struct { T data[3]; };
  };

  constexpr Vector3() = default;
  constexpr explicit Vector3(T val) : x{val}, y{val}, z{val} { }
  constexpr explicit Vector3(T val[3]) : x{val[0]}, y{val[1]}, z{val[2]} { }
  constexpr Vector3(T ax, T ay, T az) : x{ax}, y{ay}, z{az} { }
  constexpr Vector3(T ax, T ay) : x{ax}, y{ay}, z{static_cast<T>(0)} { }

  auto SqLength() const -> float { return x*x + y*y + z*z; };
  auto Length() const -> float { return std::sqrt(SqLength()); };
  auto Normalize() -> Vector3&;
  bool IsNormalized() const { return std::fabs(SqLength() - 1.f) < std::numeric_limits<float>::epsilon(); }
  void Zero() { x = 0.f; y = 0.f; z = 0.f; }
  bool IsZero() const { return std::fabs(x+y+z) < std::numeric_limits<float>::epsilon(); }

  T operator[](int i) const { return (i == 0 ? x : (i == 1 ? y : (i == 2 ? z : z))); }
  T& operator[](int i) { return (i == 0 ? x : (i == 1 ? y : (i == 2 ? z : z))); }

  Vector3 operator-()
  {
    return *this * -1.f;
  }
  Vector3& operator*=(float scalar)
  {
    this->x *= scalar;
    this->y *= scalar;
    this->z *= scalar;
    return *this;
  }
  Vector3& operator/=(float scalar)
  {
    this->x /= scalar;
    this->y /= scalar;
    this->z /= scalar;
    return *this;
  }
  Vector3& operator+=(const Vector3& rhs)
  {
    this->x += rhs.x;
    this->y += rhs.y;
    this->z += rhs.z;
    return *this;
  }
  Vector3& operator-=(const Vector3& rhs)
  {
    this->x -= rhs.x;
    this->y -= rhs.y;
    this->z -= rhs.z;
    return *this;
  }
  bool operator==(const Vector3& v)
  {
    return std::fabs(x-v.x) < std::numeric_limits<float>::epsilon() &&
           std::fabs(y-v.y) < std::numeric_limits<float>::epsilon() &&
           std::fabs(z-v.z) < std::numeric_limits<float>::epsilon();
  }
  bool operator!=(const Vector3& v)
  {
    return !(*this == v);
  }
  friend inline Vector3 operator*(Vector3 lhs, float scalar)
  {
    lhs *= scalar;
    return lhs;
  }
  friend inline Vector3 operator*(float scalar, Vector3 lhs)
  {
    lhs *= scalar;
    return lhs;
  }
  friend inline Vector3 operator/(Vector3 lhs, float scalar)
  {
    lhs /= scalar;
    return lhs;
  }
  friend inline Vector3 operator/(float scalar, Vector3 lhs)
  {
    lhs /= scalar;
    return lhs;
  }
  friend inline Vector3 operator-(Vector3 lhs, const Vector3& rhs)
  {
    lhs -= rhs;
    return lhs;
  }
  friend inline Vector3 operator+(Vector3 lhs, const Vector3& rhs)
  {
    lhs += rhs;
    return lhs;
  }
  friend inline bool operator<(const Vector3& lhs, const Vector3& rhs)
  {
    return lhs.SqLength() < rhs.SqLength();
  }
  friend inline float operator*(const Vector3& v1, const Vector3& v2) // dot
  {
    return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
  }
  friend inline Vector3 operator%(const Vector3& v1, const Vector3& v2) // cross for rhs, for lhs - invert result
  {
    return Vector3(
      (v1.y*v2.z-v2.y*v1.z), -(v1.x*v2.z-v2.x*v1.z), (v1.x*v2.y-v2.x*v1.y));
  }

}; // struct Vector3

template<class T>
using Vec3 = Vector3<T>;

using Vec3f = Vector3<float>;
using Vec3u = Vector3<unsigned int>;
using Vec3i = Vector3<int>;

namespace vec3 {

  inline float Length(const Vec3f& v) { return v.Length(); }
  inline float SqLength(const Vec3f& v) { return v.SqLength(); }
  inline Vec3f Normalize(Vec3f v) { v.Normalize(); return v; } 
  inline float DotProduct(const Vec3f& v1, const Vec3f& v2) { return v1*v2; }
  inline float Dot(const Vec3f& v1, const Vec3f& v2) { return v1*v2; }
  inline Vec3f CrossProduct(const Vec3f& v1, const Vec3f& v2) { return v1%v2; }
  inline Vec3f Cross(const Vec3f& v1, const Vec3f& v2) { return v1%v2; }
  inline Vec3f MakeX0Z(Vec3f v) { v.y = 0.f; return v; }
  inline Vec3f SwapXZ(Vec3f v) { std::swap(v.x, v.z); return v; }
  inline Vec3f Reflect(Vec3f v, const Vec3f& n) { return v-2*n*(n*v); }

  template<class T>
  void InUpperBound(Vector3<T>&, float upper);
  template<class T>
  auto ValuePtr(const Vector3<T>& vec) -> const T* { return &vec.x; }

} // namespace vec3

template<class T>
inline Vector3<T>& Vector3<T>::Normalize()
{
  if (IsNormalized())
    return *this;
  float len = Length();
  if (std::fabs(len) < std::numeric_limits<float>::epsilon())
    return *this;
  float p = static_cast<T>(1) / len;
  x *= p;
  y *= p;
  z *= p;
  return *this;
}

// Make vector in range (-upper; +upper)

template<class T>
inline void vec3::InUpperBound(Vector3<T>& v, float upper)
{
  if (v.x >= upper || v.x <= -upper)
    v.x = std::fmod(v.x, upper);
  if (v.y >= upper || v.y <= -upper)  
    v.y = std::fmod(v.y, upper);
  if (v.z >= upper || v.z <= -upper)  
    v.z = std::fmod(v.z, upper);
}

} // namespace gdm

#endif  // AH_GDM_VECTOR3_H
