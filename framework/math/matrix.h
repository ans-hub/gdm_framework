// *************************************************************
// File:    matrix.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

// Description: col-major matrix with pre-multiplication (i.e. p*v*m)
//  all rotation matrix therefore need to be premultiplied or transposed before postmul
// Cartesian cs: OpenGL - RH (-z fwd). DirectX - LH (+z fwd)
// Ndc: OpenGL - [-1;+1], DirectX - [0;+1]
// Matrices: OpenGl, Dx10 && later - Col major (pre-mp), DirectX 9 - Row major 
// Math: a e g x  row_major (means in array we are storing mx rows): aegx bfiy cgjz dhk1
//       b f i y  col_major (means in array we are storing mx cols): abcd efgh gijk xyz1
//       c g j z  with col major we need to either post-multiplication or transpose mxs
//       d h k 1

#ifndef AH_GDM_MATRIX_H
#define AH_GDM_MATRIX_H

#ifndef GDM_MATRIX_INV_TRANSPOSE_TYPE
#define GDM_MATRIX_INV_TRANSPOSE_TYPE 1
#endif

#ifndef GDM_DIRECT_X
#define GDM_DIRECT_X 1
#endif

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-anonymous-struct"
#endif

#include <array>
#include <cmath>
#include <algorithm>
#include <initializer_list>
#include <cassert>

#include "math/vector2.h"
#include "math/vector3.h"

namespace gdm {

template<std::size_t Row, std::size_t Col>
struct Matrix
{
  using Container = std::array<float, Row * Col>;
  Container data_;

public:
  Matrix() =default;
  explicit Matrix(float diagonal_value);
  Matrix(const std::initializer_list<float>&);
  Matrix(const Matrix&) =default;
  Matrix& operator=(const Matrix&) =default;
  Matrix(Matrix&&) =default;
  Matrix& operator=(Matrix&&) =default;

  auto Size() const -> std::size_t { return data_.count(); }
  auto Data() -> Container&{ return data_; }
  auto Data() const -> const Container&{ return data_; }

  auto GetCol(std::size_t col) const -> Vec3f;
  auto GetColRef(std::size_t col) -> Vec3f&;
  auto GetColRef(std::size_t col) const -> const Vec3f&;
  void SetCol(std::size_t col, const Vec3f&);

  Vec3f operator*(const Vec3f&);
  Vec3f operator%(const Vec3f&);
  Matrix operator*(const Matrix& rhs);
  Matrix operator%(const Matrix& rhs);

  Matrix& operator*=(float s);
  Matrix& operator/=(float s);
  Matrix& operator+=(const Matrix& rhs);
  Matrix& operator-=(const Matrix& rhs);
  float operator()(std::size_t i, std::size_t k) const { return data_[i * Col + k]; }
  float& operator()(std::size_t i, std::size_t k) { return data_[i * Col + k]; }
  float operator[](std::size_t i) const { return data_[i]; }
  float& operator[](std::size_t i) { return data_[i]; }
  friend inline Matrix operator*(Matrix m, float s) { m *= s; return m; }
  friend inline Matrix operator/(Matrix m, float s) { m /= s; return m; }
  friend inline Matrix operator-(Matrix m, float s) { m -= s; return m; }
  friend inline Matrix operator+(Matrix m, float s) { m += s; return m; }

}; // struct Matrix

using Mat2f = Matrix<2,2>;
using Mat3f = Matrix<3,3>;
using Mat4f = Matrix<4,4>;

namespace matrix {

  // Helper functions for general matrices

  template<std::size_t R1, std::size_t R2, std::size_t C>   
  Matrix<R1,R2> Multiplie(const Matrix<R1,C>&, const Matrix<C,R2>&);
  template<std::size_t R, std::size_t C> 
  Matrix<C,R> Transpose(const Matrix<R,C>&);  

  // Helper functions for transformation matrices

  float Determinant(const Mat2f& mx);
  float Determinant(const Mat3f& mx);
  float Determinant(const Mat4f& mx);
  float DeterminantRot(const Mat4f& mx);
  Mat2f Inverse(const Mat2f& mx);
  Mat3f Inverse(const Mat3f& mx);
  Mat4f Inverse(const Mat4f& mx);
  Mat4f InverseTransform(const Mat4f& mt);
  Vec2f Multiplie(const Vec2f& v, const Mat3f& mx);
  Vec3f Multiplie(const Vec3f& v, const Mat4f& mx);
  Vec3f MultiplieDirectional(const Vec3f& v, const Mat4f& mx);
  Mat3f Transpose(const Mat3f& lhs, const Mat3f& rhs);
  Mat4f Transpose(const Mat4f& lhs, const Mat4f& rhs);
  Mat4f MakeRotateX(float angle);
  Mat4f MakeRotateY(float angle);
  Mat4f MakeRotateZ(float angle);
  Mat4f MakeRotate(float angle, Vec3f axis);
  Mat4f MakePerspectiveRH(float fov_deg, float ar, float z_near, float z_far);
  Mat4f MakePerspectiveLH(float fov_deg, float ar, float z_near, float z_far);
  Mat4f MakeOrthoLH(float width, float height, float z_near, float z_far);
  Mat4f MakeOrthoRH(float width, float height, float z_near, float z_far);
  Mat4f MakeTranslate(const Vec3f& pos);
  Mat4f MakeScale(float factor);
  Mat4f MakeNormalMatrix(const Mat4f& mx);
  void  MakeIdentity(Mat2f& mx);
  void  MakeIdentity(Mat3f& mx);
  void  MakeIdentity(Mat4f& mx);
  void  MakeLookAt(Mat4f& mx, const Vec3f& look_at, Vec3f look_from, Vec3f up_dir);
  void  Orthonormalize(Mat4f& mx);
  auto  DecomposeToScale(const Mat4f& mx) -> Vec3f;
  
  Mat4f ClearOrient(const Mat4f& mx);
  Mat4f MakeLHBasis();
  Mat4f MakeRHBasis();
  Mat4f SwapForwardRight(Mat4f mx);

  // Other helpers

  template<std::size_t R, std::size_t C>
    const float* ValuePtr(const Matrix<R,C>& mx) { return &(mx.Data()[0]); }
  template<class T>
    void PrintValues(const T* ptr, int sz);
  template<class T, class...Args>
    T& Make(Args&&...args){ return T(std::forward<Args>(args)...); }

} // namespace matrix

// Multiplie matrixes (naive) 
// todo: due to i've changed mp order to correct (in other funcs)
// current mul in this function may be incorrect. Need to make col*row mp

template<std::size_t R1, std::size_t R2, std::size_t C>   
inline Matrix<R1,R2> matrix::Multiplie(const Matrix<R1,C>& mx1, const Matrix<C,R2>& mx2)
{
  Matrix<R1,R2> res {};

  for(std::size_t row = 0; row < res.Rows(); ++row)
    for(std::size_t col = 0; col < res.Cols(); ++col)
      for(std::size_t inner = 0; inner < mx1.Cols(); ++inner)
        res(row,col) += mx1(row,inner) * mx2(inner,col);  //mx1_row * mx2_col

  return res;
}

// Special fast multiplie function Mat3f

template<>
inline Mat3f matrix::Multiplie(const Mat3f& l, const Mat3f& r)
{
  return {
    l(0,0) * r(0,0) + l(1,0) * r(0,1) + l(2,0) * r(0,2),
    l(0,1) * r(0,0) + l(1,1) * r(0,1) + l(2,1) * r(0,2),
    l(0,2) * r(0,0) + l(1,2) * r(0,1) + l(2,2) * r(0,2),
    l(0,0) * r(1,0) + l(1,0) * r(1,1) + l(2,0) * r(1,2),
    l(0,1) * r(1,0) + l(1,1) * r(1,1) + l(2,1) * r(1,2),
    l(0,2) * r(1,0) + l(1,2) * r(1,1) + l(2,2) * r(1,2),
    l(0,0) * r(2,0) + l(1,0) * r(2,1) + l(2,0) * r(2,2),
    l(0,1) * r(2,0) + l(1,1) * r(2,1) + l(2,1) * r(2,2),
    l(0,2) * r(2,0) + l(1,2) * r(2,1) + l(2,2) * r(2,2)
  };
}

// Special fast multiplie function Mat4f (so we multiplie row to columns,
// and as this is col major layout we take rows from columns of array, 
// and vice versa)

template<>
inline Mat4f matrix::Multiplie(const Mat4f& l, const Mat4f& r)
{
  // return {
  //   l(0,0) * r(0,0) + l(0,1) * r(1,0) + l(0,2) * r(2,0) + l(0,3) * r(3,0),
  //   l(0,0) * r(0,1) + l(0,1) * r(1,1) + l(0,2) * r(2,1) + l(0,3) * r(3,1),
  //   l(0,0) * r(0,2) + l(0,1) * r(1,2) + l(0,2) * r(2,2) + l(0,3) * r(3,2),
  //   l(0,0) * r(0,3) + l(0,1) * r(1,3) + l(0,2) * r(2,3) + l(0,3) * r(3,3),
  //   l(1,0) * r(0,0) + l(1,1) * r(1,0) + l(1,2) * r(2,0) + l(1,3) * r(3,0),
  //   l(1,0) * r(0,1) + l(1,1) * r(1,1) + l(1,2) * r(2,1) + l(1,3) * r(3,1),
  //   l(1,0) * r(0,2) + l(1,1) * r(1,2) + l(1,2) * r(2,2) + l(1,3) * r(3,2),
  //   l(1,0) * r(0,3) + l(1,1) * r(1,3) + l(1,2) * r(2,3) + l(1,3) * r(3,3),
  //   l(2,0) * r(0,0) + l(2,1) * r(1,0) + l(2,2) * r(2,0) + l(2,3) * r(3,0),
  //   l(2,0) * r(0,1) + l(2,1) * r(1,1) + l(2,2) * r(2,1) + l(2,3) * r(3,1),
  //   l(2,0) * r(0,2) + l(2,1) * r(1,2) + l(2,2) * r(2,2) + l(2,3) * r(3,2),
  //   l(2,0) * r(0,3) + l(2,1) * r(1,3) + l(2,2) * r(2,3) + l(2,3) * r(3,3),
  //   l(3,0) * r(0,0) + l(3,1) * r(1,0) + l(3,2) * r(2,0) + l(3,3) * r(3,0),
  //   l(3,0) * r(0,1) + l(3,1) * r(1,1) + l(3,2) * r(2,1) + l(3,3) * r(3,1),
  //   l(3,0) * r(0,2) + l(3,1) * r(1,2) + l(3,2) * r(2,2) + l(3,3) * r(3,2),
  //   l(3,0) * r(0,3) + l(3,1) * r(1,3) + l(3,2) * r(2,3) + l(3,3) * r(3,3)
  // };
  return {
    l(0,0) * r(0,0) + l(1,0) * r(0,1) + l(2,0) * r(0,2) + l(3,0) * r(0,3),
    l(0,1) * r(0,0) + l(1,1) * r(0,1) + l(2,1) * r(0,2) + l(3,1) * r(0,3),
    l(0,2) * r(0,0) + l(1,2) * r(0,1) + l(2,2) * r(0,2) + l(3,2) * r(0,3),
    l(0,3) * r(0,0) + l(1,3) * r(0,1) + l(2,3) * r(0,2) + l(3,3) * r(0,3),
    l(0,0) * r(1,0) + l(1,0) * r(1,1) + l(2,0) * r(1,2) + l(3,0) * r(1,3),
    l(0,1) * r(1,0) + l(1,1) * r(1,1) + l(2,1) * r(1,2) + l(3,1) * r(1,3),
    l(0,2) * r(1,0) + l(1,2) * r(1,1) + l(2,2) * r(1,2) + l(3,2) * r(1,3),
    l(0,3) * r(1,0) + l(1,3) * r(1,1) + l(2,3) * r(1,2) + l(3,3) * r(1,3),
    l(0,0) * r(2,0) + l(1,0) * r(2,1) + l(2,0) * r(2,2) + l(3,0) * r(2,3),
    l(0,1) * r(2,0) + l(1,1) * r(2,1) + l(2,1) * r(2,2) + l(3,1) * r(2,3),
    l(0,2) * r(2,0) + l(1,2) * r(2,1) + l(2,2) * r(2,2) + l(3,2) * r(2,3),
    l(0,3) * r(2,0) + l(1,3) * r(2,1) + l(2,3) * r(2,2) + l(3,3) * r(2,3),
    l(0,0) * r(3,0) + l(1,0) * r(3,1) + l(2,0) * r(3,2) + l(3,0) * r(3,3),
    l(0,1) * r(3,0) + l(1,1) * r(3,1) + l(2,1) * r(3,2) + l(3,1) * r(3,3),
    l(0,2) * r(3,0) + l(1,2) * r(3,1) + l(2,2) * r(3,2) + l(3,2) * r(3,3),
    l(0,3) * r(3,0) + l(1,3) * r(3,1) + l(2,3) * r(3,2) + l(3,3) * r(3,3)
  };
}

template<std::size_t R, std::size_t C>
Matrix<R,C>::Matrix(float diagonal_value)
  : data_()
{
  for (std::size_t i = 0; i < R; ++i)
    data_[i * C + i] = diagonal_value;
}

template<std::size_t R, std::size_t C>
Matrix<R,C>::Matrix(const std::initializer_list<float>& list)
  : data_{}
{
  assert(data_.size() == list.size());
  int i {0};
  for (auto& item : list)
    data_[i++] = item;
}

template<>
inline Vec3f Mat4f::GetCol(std::size_t c) const
{
  const std::size_t r_ {4};
  assert(c < 4);
  return Vec3f(
    data_[r_*c+0], data_[r_*c+1], data_[r_*c+2]);
}

template<>
inline Vec3f& Mat4f::GetColRef(std::size_t c)
{
  const std::size_t r_ {4};
  assert(c < 4);
  return reinterpret_cast<Vec3f&>(data_[r_*c+0]);
}

template<>
inline const Vec3f& Mat4f::GetColRef(std::size_t c) const
{
  const std::size_t r_ {4};
  assert(c < 4);
  return reinterpret_cast<const Vec3f&>(data_[r_*c+0]);
}

template<>
inline void Mat4f::SetCol(std::size_t col, const Vec3f& v)
{
  const std::size_t r_ {4};
  data_[r_*col+0] = v.x;
  data_[r_*col+1] = v.y;
  data_[r_*col+2] = v.z;
  data_[r_*col+3] = col == 3 ? 1.f : 0.f;
}

// Rotate and translate vector by matrix (see note after this code section)

template<>
inline Vec3f Mat4f::operator*(const Vec3f& v)
{
  return matrix::Multiplie(v, *this);
}

// Rotate without translating vector by matrix

template<>
inline Vec3f Mat4f::operator%(const Vec3f& v)
{
  return matrix::MultiplieDirectional(v, *this);
}

template<std::size_t R, std::size_t C>
inline Matrix<R,C> Matrix<R,C>::operator*(const Matrix<R,C>& rhs)
{
  return matrix::Multiplie(*this, rhs);
}

template<>   
inline Mat3f Mat3f::operator*(const Mat3f& rhs)
{
  return matrix::Multiplie(*this, rhs);
}

// Note: as it column row matrix - we premul matrixces, so store pos of rhs

template<>
inline Mat4f Mat4f::operator%(const Mat4f& rhs)
{
  Vec3f pos = rhs.GetCol(3);
  Mat4f res = matrix::Multiplie(*this, rhs);
  res.SetCol(3, pos);
  return res;
}

template<std::size_t R, std::size_t C>
inline Matrix<R,C>& Matrix<R,C>::operator*=(float s)
{
  std::for_each (
    data_.begin(), data_.end(), [&s](float& e){ e *= s; }
  );
  return *this;
}

template<std::size_t R, std::size_t C>
inline Matrix<R,C>& Matrix<R,C>::operator/=(float s)
{
  assert(std::fabs(s) > std::numeric_limits<float>::epsilon());
  std::for_each (
    data_.begin(), data_.end(), [&s](float& e){ e /= s; }
  );
  return *this;
}

template<std::size_t R, std::size_t C>   
inline Matrix<R,C>& Matrix<R,C>::operator+=(const Matrix& rhs)
{
  std::transform
  (
    data_.begin(), data_.end(), rhs.data_.begin(), data_.begin(),
    std::plus<float>()
  );
  return *this;
}

template<std::size_t R, std::size_t C>   
inline Matrix<R,C>& Matrix<R,C>::operator-=(const Matrix& rhs)
{
  std::transform (
    data_.begin(), data_.end(), rhs.data_.begin(), data_.begin(),
    std::minus<float>()
  );
  return *this;
}

template<std::size_t R, std::size_t C> 
inline Matrix<C,R> matrix::Transpose(const Matrix<R,C>& mx)
{
  Matrix<C,R> res {};
  for (std::size_t i = 0; i < R; ++i)
  {
    for (std::size_t k = 0; k < C; ++k)
      res(k,i) = mx(i,k);
  }
  return res;
}

template<class T>
inline void matrix::PrintValues(const T* ptr, int sz)
{
  for (int i = 0; i < sz; ++i)
    std::printf("%.02f ", *(ptr++));
  std::printf("\n");
}

inline float matrix::Determinant(const Mat2f& mx)
{
  return (
    mx(0,0) * mx(1,1) - mx(0,1) * mx(1,0)
  );
}

inline float matrix::Determinant(const Mat3f& mx)
{
  return (
    mx(0,0) * (mx(1,1) * mx(2,2) - mx(2,1) * mx(1,2)) - 
    mx(0,1) * (mx(1,0) * mx(2,2) - mx(2,0) * mx(1,2)) + 
    mx(0,2) * (mx(1,0) * mx(2,1) - mx(2,0) * mx(1,1))
  );
}

inline float matrix::Determinant(const Mat4f& m)
{
  return (
    m(0,3) * m(1,2) * m(2,1) * m(3,0) - m(0,2) * m(1,3) * m(2,1) * m(3,0) -
    m(0,3) * m(1,1) * m(2,2) * m(3,0) + m(0,1) * m(1,3) * m(2,2) * m(3,0) +
    m(0,2) * m(1,1) * m(2,3) * m(3,0) - m(0,1) * m(1,2) * m(2,3) * m(3,0) -
    m(0,3) * m(1,2) * m(2,0) * m(3,1) + m(0,2) * m(1,3) * m(2,0) * m(3,1) +
    m(0,3) * m(1,0) * m(2,2) * m(3,1) - m(0,0) * m(1,3) * m(2,2) * m(3,1) -
    m(0,2) * m(1,0) * m(2,3) * m(3,1) + m(0,0) * m(1,2) * m(2,3) * m(3,1) +
    m(0,3) * m(1,1) * m(2,0) * m(3,2) - m(0,1) * m(1,3) * m(2,0) * m(3,2) -
    m(0,3) * m(1,0) * m(2,1) * m(3,2) + m(0,0) * m(1,3) * m(2,1) * m(3,2) +
    m(0,1) * m(1,0) * m(2,3) * m(3,2) - m(0,0) * m(1,1) * m(2,3) * m(3,2) -
    m(0,2) * m(1,1) * m(2,0) * m(3,3) + m(0,1) * m(1,2) * m(2,0) * m(3,3) +
    m(0,2) * m(1,0) * m(2,1) * m(3,3) - m(0,0) * m(1,2) * m(2,1) * m(3,3) -
    m(0,1) * m(1,0) * m(2,2) * m(3,3) + m(0,0) * m(1,1) * m(2,2) * m(3,3)
  );
}

// Returns determinant of 3x3 rotation part of 4x4 matrix

inline float matrix::DeterminantRot(const Mat4f& mx)
{
  return (
    mx(0,0) * (mx(1,1) * mx(2,2) - mx(2,1) * mx(1,2)) - 
    mx(0,1) * (mx(1,0) * mx(2,2) - mx(2,0) * mx(1,2)) + 
    mx(0,2) * (mx(1,0) * mx(2,1) - mx(2,0) * mx(1,1))
  );
}

inline Mat2f matrix::Inverse(const Mat2f& mx)
{
  Mat2f res (1.f);
  float det = matrix::Determinant(mx);
  float inv = 1.f / det;

  assert(std::fabs(det) > std::numeric_limits<float>::epsilon());
  res(0,0) =  mx(1,1) * inv;
  res(0,1) = -mx(0,1) * inv;
  res(1,0) = -mx(1,0) * inv;
  res(1,1) =  mx(0,0) * inv;
  return res;
}

inline Mat3f matrix::Inverse(const Mat3f& mx)
{
  Mat3f res (1.f);
  float det = matrix::Determinant(mx);
  float inv = 1.f / det;

  assert(std::fabs(det) > std::numeric_limits<float>::epsilon());

  res(0,0) = (mx(1,1) * mx(2,2) - mx(2,1) * mx(1,2)) * inv;
  res(0,1) = (mx(0,1) * mx(2,2) - mx(2,1) * mx(0,2)) * (-inv);
  res(0,2) = (mx(0,1) * mx(1,2) - mx(1,1) * mx(0,2)) * inv;

  res(1,0) = (mx(1,0) * mx(2,2) - mx(2,0) * mx(1,2)) * (-inv);
  res(1,1) = (mx(0,0) * mx(2,2) - mx(2,0) * mx(0,2)) * inv;
  res(1,2) = (mx(0,0) * mx(1,2) - mx(1,0) * mx(0,2)) * (-inv);

  res(2,0) = (mx(1,0) * mx(2,1) - mx(2,0) * mx(1,1)) * inv;
  res(2,1) = (mx(0,0) * mx(2,1) - mx(2,0) * mx(0,1)) * (-inv);
  res(2,2) = (mx(0,0) * mx(1,1) - mx(1,0) * mx(0,1)) * inv;

  return res;
}

inline Mat4f matrix::Inverse(const Mat4f& m)
{
  Mat4f res (1.f);

  res(0,0) =  m(1,1) * m(2,2) * m(3,3) - m(1,1) * m(2,3) * m(3,2) -
              m(2,1) * m(1,2) * m(3,3) + m(2,1) * m(1,3) * m(3,2) +
              m(3,1) * m(1,2) * m(2,3) - m(3,1) * m(1,3) * m(2,2);
  res(1,0) = -m(1,0) * m(2,2) * m(3,3) + m(1,0) * m(2,3) * m(3,2) +
              m(2,0) * m(1,2) * m(3,3) - m(2,0) * m(1,3) * m(3,2) -
              m(3,0) * m(1,2) * m(2,3) + m(3,0) * m(1,3) * m(2,2);
  res(2,0) =  m(1,0) * m(2,1) * m(3,3) - m(1,0) * m(2,3) * m(3,1) -
              m(2,0) * m(1,1) * m(3,3) + m(2,0) * m(1,3) * m(3,1) +
              m(3,0) * m(1,1) * m(2,3) - m(3,0) * m(1,3) * m(2,1);
  res(3,0) = -m(1,0) * m(2,1) * m(3,2) + m(1,0) * m(2,2) * m(3,1) +
              m(2,0) * m(1,1) * m(3,2) - m(2,0) * m(1,2) * m(3,1) -
              m(3,0) * m(1,1) * m(2,2) + m(3,0) * m(1,2) * m(2,1);
  res(0,1) = -m(0,1) * m(2,2) * m(3,3) + m(0,1) * m(2,3) * m(3,2) +
              m(2,1) * m(0,2) * m(3,3) - m(2,1) * m(0,3) * m(3,2) -
              m(3,1) * m(0,2) * m(2,3) + m(3,1) * m(0,3) * m(2,2);
  res(1,1) =  m(0,0) * m(2,2) * m(3,3) - m(0,0) * m(2,3) * m(3,2) -
              m(2,0) * m(0,2) * m(3,3) + m(2,0) * m(0,3) * m(3,2) +
              m(3,0) * m(0,2) * m(2,3) - m(3,0) * m(0,3) * m(2,2);
  res(2,1) = -m(0,0) * m(2,1) * m(3,3) + m(0,0) * m(2,3) * m(3,1) +
              m(2,0) * m(0,1) * m(3,3) - m(2,0) * m(0,3) * m(3,1) -
              m(3,0) * m(0,1) * m(2,3) + m(3,0) * m(0,3) * m(2,1);
  res(3,1) =  m(0,0) * m(2,1) * m(3,2) - m(0,0) * m(2,2) * m(3,1) -
              m(2,0) * m(0,1) * m(3,2) + m(2,0) * m(0,2) * m(3,1) +
              m(3,0) * m(0,1) * m(2,2) - m(3,0) * m(0,2) * m(2,1);
  res(0,2) =  m(0,1) * m(1,2) * m(3,3) - m(0,1) * m(1,3) * m(3,2) -
              m(1,1) * m(0,2) * m(3,3) + m(1,1) * m(0,3) * m(3,2) +
              m(3,1) * m(0,2) * m(1,3) - m(3,1) * m(0,3) * m(1,2);
  res(1,2) = -m(0,0) * m(1,2) * m(3,3) + m(0,0) * m(1,3) * m(3,2) +
              m(1,0) * m(0,2) * m(3,3) - m(1,0) * m(0,3) * m(3,2) -
              m(3,0) * m(0,2) * m(1,3) + m(3,0) * m(0,3) * m(1,2);
  res(2,2) =  m(0,0) * m(1,1) * m(3,3) - m(0,0) * m(1,3) * m(3,1) -
              m(1,0) * m(0,1) * m(3,3) + m(1,0) * m(0,3) * m(3,1) +
              m(3,0) * m(0,1) * m(1,3) - m(3,0) * m(0,3) * m(1,1);
  res(3,2) = -m(0,0) * m(1,1) * m(3,2) + m(0,0) * m(1,2) * m(3,1) +
              m(1,0) * m(0,1) * m(3,2) - m(1,0) * m(0,2) * m(3,1) -
              m(3,0) * m(0,1) * m(1,2) + m(3,0) * m(0,2) * m(1,1);
  res(0,3) = -m(0,1) * m(1,2) * m(2,3) + m(0,1) * m(1,3) * m(2,2) +
              m(1,1) * m(0,2) * m(2,3) - m(1,1) * m(0,3) * m(2,2) -
              m(2,1) * m(0,2) * m(1,3) + m(2,1) * m(0,3) * m(1,2);
  res(1,3) =  m(0,0) * m(1,2) * m(2,3) - m(0,0) * m(1,3) * m(2,2) -
              m(1,0) * m(0,2) * m(2,3) + m(1,0) * m(0,3) * m(2,2) +
              m(2,0) * m(0,2) * m(1,3) - m(2,0) * m(0,3) * m(1,2);
  res(2,3) = -m(0,0) * m(1,1) * m(2,3) + m(0,0) * m(1,3) * m(2,1) +
              m(1,0) * m(0,1) * m(2,3) - m(1,0) * m(0,3) * m(2,1) -
              m(2,0) * m(0,1) * m(1,3) + m(2,0) * m(0,3) * m(1,1);
  res(3,3) =  m(0,0) * m(1,1) * m(2,2) - m(0,0) * m(1,2) * m(2,1) -
              m(1,0) * m(0,1) * m(2,2) + m(1,0) * m(0,2) * m(2,1) +
              m(2,0) * m(0,1) * m(1,2) - m(2,0) * m(0,2) * m(1,1);

  float det = m(0,0) * res(0,0) + m(0,1) * res(1,0) +
              m(0,2) * res(2,0) + m(0,3) * res(3,0);

  assert(std::fabs(det) > std::numeric_limits<float>::epsilon());

  det = 1.f / det;

  for(auto& element : res.Data())
    element *= det;
  return res;
}

// Functions for inverting transformation matrices (only orthonormalized)

#if GDM_MATRIX_INV_TRANSPOSE_TYPE == 1

// Makes inverted transform matrix (by multiplying 2 inverted matrices)

inline Mat4f matrix::InverseTransform(const Mat4f& tm)
{
  Mat4f itm (1.f);
  itm.SetCol(0, tm.GetCol(0));
  itm.SetCol(1, tm.GetCol(1));
  itm.SetCol(2, tm.GetCol(2));
  itm = matrix::Transpose(itm);
  Mat4f t = matrix::MakeTranslate(tm.GetCol(3) * -1.f);
  itm = itm * t;
  return itm;
}

#else

// Makes inverted transform matrix (by transposing orientation and applying
// inverted translate)

inline Mat4f matrix::InverseTransform(const Mat4f& tm)
{
  Mat4f itm (1.f);
  itm.SetCol(0, tm.GetCol(0));
  itm.SetCol(1, tm.GetCol(1));
  itm.SetCol(2, tm.GetCol(2));
  itm = matrix::Transpose(itm);
  Vec3f itm_pos = itm % (tm.GetCol(3) * -1.f);
  itm.SetCol(3, itm_pos);
  return itm;
}

#endif

// Interpret vector as 3d vector with ficted w=1

inline Vec2f matrix::Multiplie(const Vec2f& v, const Mat3f& mx)
{
  Vec2f res;
  res.x = v.x * mx(0,0) + v.y * mx(1,0) + mx(2,0);
  res.y = v.x * mx(0,1) + v.y * mx(1,1) + mx(2,1);
  return res;
}

// Interpret vector as 4d vector (with w=1) and translate its position

inline Vec3f matrix::Multiplie(const Vec3f& v, const Mat4f& mx)
{
  Vec3f res {};
  res.x = v.x * mx(0,0) + v.y * mx(1,0) + v.z * mx(2,0) + mx(3,0);
  res.y = v.x * mx(0,1) + v.y * mx(1,1) + v.z * mx(2,1) + mx(3,1);
  res.z = v.x * mx(0,2) + v.y * mx(1,2) + v.z * mx(2,2) + mx(3,2);
  return res;
}

// Interpret vector as 4d vector (with w=0) without translating its position

inline Vec3f matrix::MultiplieDirectional(const Vec3f& v, const Mat4f& mx)
{
  Vec3f res {};
  res.x = v.x * mx(0,0) + v.y * mx(1,0) + v.z * mx(2,0);
  res.y = v.x * mx(0,1) + v.y * mx(1,1) + v.z * mx(2,1);
  res.z = v.x * mx(0,2) + v.y * mx(1,2) + v.z * mx(2,2);
  return res;
}

inline Mat4f matrix::MakeRotateX(float angle)
{
  float rad = angle * 3.1415926535f / 180.0f;
  float tsin = std::sin(rad);
  float tcos = std::cos(rad);
  return {
    1.0f,   0.0f,   0.0f,   0.0f,
    0.0f,   tcos,   tsin,   0.0f, 
    0.0f,  -tsin,   tcos,   0.0f,
    0.0f,   0.0f,   0.0f,   1.0f 
  };
}

inline Mat4f matrix::MakeRotateY(float angle)
{
  float rad = angle * 3.1415926535f / 180.0f;
  float tsin = std::sin(rad);
  float tcos = std::cos(rad);
  return {
    tcos,   0.0f,  -tsin,   0.0f,
    0.0f,   1.0f,   0.0f,   0.0f, 
    tsin,   0.0f,   tcos,   0.0f,
    0.0f,   0.0f,   0.0f,   1.0f 
  };
}

inline Mat4f matrix::MakeRotateZ(float angle)
{
  float rad = angle * 3.1415926535f / 180.0f;
  float tsin = std::sin(rad);
  float tcos = std::cos(rad);
  return {
    tcos,   tsin,   0.0f,   0.0f,
    -tsin,  tcos,   0.0f,   0.0f, 
    0.0f,   0.0f,   1.0f,   0.0f,
    0.0f,   0.0f,   0.0f,   1.0f 
  };
}

// Returns rotation matrix about arbitrary axis

inline Mat4f matrix::MakeRotate(float angle_deg, Vec3f axis)
{
  float angle = angle_deg * 3.1415926535f / 180.f;
  float tsin = std::sin(angle);
  float tcos = std::cos(angle);

  axis.Normalize();
  Vec3f tmp {axis * (1.f - tcos)};    
  Mat4f res (1.f);

  res(0,0) = tcos + tmp.x * axis.x;
  res(0,1) = tmp.x * axis.y + tsin * axis.z;
  res(0,2) = tmp.x * axis.z - tsin * axis.y;

  res(1,0) = tmp.y * axis.x - tsin * axis.z;
  res(1,1) = tcos + tmp.y * axis.y;
  res(1,2) = tmp.y * axis.z + tsin * axis.x;

  res(2,0) = tmp.z * axis.x + tsin * axis.y;
  res(2,1) = tmp.z * axis.y - tsin * axis.x;
  res(2,2) = tcos + tmp.z * axis.z;

  return res;
}

// Returns perspective matrix for RH (opengl usually)
// Diff for dx and opengl - [0;1] && [-1;1]

// http://xdpixel.com/decoding-a-projection-matrix/
// https://www.gamedev.net/articles/programming/graphics/perspective-projections-in-lh-and-rh-systems-r3598/

inline Mat4f matrix::MakePerspectiveRH(float fov_deg, float ar, float z_near, float z_far)
{
  Mat4f res (0.f);

  float fov = fov_deg * 3.1415926535f / 180.f;
  float tan = std::tan(fov / 2.f);
#if GDM_DIRECT_X == 1
  res(0,0) = 1.f / (ar * tan);
  res(1,1) = 1.f / tan;
  res(2,2) = z_far / (z_far - z_near);
  res(2,3) = -1.f;
  res(3,2) = -(z_near * z_far) / (z_far - z_near);
#else
  // res(0,0) = 1.f / (ar * tan);
  res(0,0) = ar / tan;            // todo: check on opengl
  res(0,0) = 1.f / (ar * tan);
  res(2,2) = -(z_far + z_near) / (z_far - z_near);
  res(2,3) = -1.f;
  res(3,2) = -(2.f * z_far * z_near) / (z_far - z_near);
#endif
  return res;
}

// Returns perspective matrix for LH (directx usually)
// Diff for dx and opengl - [0;1] && [-1;1]

// http://xdpixel.com/decoding-a-projection-matrix/
// https://www.gamedev.net/articles/programming/graphics/perspective-projections-in-lh-and-rh-systems-r3598/

inline Mat4f matrix::MakePerspectiveLH(float fov_deg, float ar, float z_near, float z_far)
{
  Mat4f res (0.f);

  float fov = fov_deg * 3.1415926535f / 180.f;
  float tan = std::tan(fov / 2.f);
#if GDM_DIRECT_X == 1
  res(0,0) = 1.f / (ar * tan);
  res(1,1) = 1.f / tan;
  res(2,2) = z_far / (z_far - z_near);
  res(2,3) = 1.f;
  res(3,2) = -(z_near * z_far) / (z_far - z_near);
#else
  res(0,0) = 1.f / (ar * tan);
  res(1,1) = 1.f / tan;
  res(2,2) = -(z_far + z_near) / (z_far - z_near);
  res(2,3) = 1.f;
  res(3,2) = (2.f * z_far * z_near) / (z_far - z_near);
#endif
  return res;
}
inline Mat4f matrix::MakeOrthoLH(float width, float height, float z_near, float z_far)
{
  Mat4f res (1.f);

#if GDM_DIRECT_X == 1
  float d = z_far - z_near;
  res(0,0) = 2.0f / width;
  res(1,1) = 2.0f / height;
  res(2,2) = 1.0f / d;
  res(3,2) = -z_near / d;
#else
  assert(false && "MakeOrthoLH not implemented");
#endif
  return res;
}

inline Mat4f matrix::MakeOrthoRH(float width, float height, float z_near, float z_far)
{
  Mat4f res (1.f);

#if GDM_DIRECT_X == 1
  float d = z_near - z_far;
  res(0,0) = 2.0f / width;
  res(1,1) = 2.0f / height;
  res(2,2) = 1.0f / d;
  res(3,2) = z_near / d;
#else
  assert(false && "MakeOrthoRH not implemented");
#endif
  return res;
}

inline Mat4f matrix::MakeTranslate(const Vec3f& pos)
{
  Mat4f res (1.f);
  res.SetCol(3, pos);
  return res;
}

inline Mat4f matrix::MakeScale(float factor)
{
  Mat4f res {};
  res(0,0) = res(1,1) = res(2,2) = factor;
  res(3,3) = 1.f;
  return res;
}

inline void matrix::MakeIdentity(Mat2f& mx)
{
  mx(0,0) = 1.f; mx(1,1) = 1.f;
}

inline void matrix::MakeIdentity(Mat3f& mx)
{
  mx(0,0) = 1.f; mx(1,1) = 1.f; mx(2,2) = 1.f;
}

inline void matrix::MakeIdentity(Mat4f& mx)
{
  mx(0,0) = 1.f; mx(1,1) = 1.f; mx(2,2) = 1.f; mx(3,3) = 1.f;
}

inline Mat4f matrix::ClearOrient(const Mat4f& in)
{
  Mat4f mx (1.f);
  mx(0,0) = 1.f; mx(1,1) = 1.f; mx(2,2) = 1.f;
  mx.SetCol(3, in.GetCol(3));
  return mx;
}

inline void matrix::MakeLookAt(Mat4f& mx, const Vec3f& look_at, Vec3f look_from, Vec3f up_dir)
{
  assert(vec3::SqLength(up_dir) > std::numeric_limits<float>::epsilon());

  Vec3f fwd = vec3::Normalize(look_at - look_from);
  Vec3f right = -vec3::Normalize(fwd % up_dir);
  up_dir = -vec3::Normalize(right % fwd);

  mx.SetCol(0, right);
  mx.SetCol(1, up_dir);
  mx.SetCol(2, fwd);
  mx.SetCol(3, look_from);
}

// Removes share and scale from matrix

inline Mat4f matrix::MakeNormalMatrix(const Mat4f& mx)
{
  return matrix::Transpose(matrix::Inverse(mx));
}

// Gram-Schmidt algorithm used - https://nptel.ac.in/courses/122104018/node49.html

inline void matrix::Orthonormalize(Mat4f& mx)
{
  Vec3f fwd = vec3::Normalize(mx.GetCol(0));
  Vec3f up = vec3::Normalize(mx.GetCol(2));
  fwd = vec3::Normalize(fwd - up * (fwd * up)); // re-orthogonalize fwd relative to up
  Vec3f right = up % fwd;
  mx.SetCol(0, right);
  mx.SetCol(1, up);
  mx.SetCol(2, fwd);
}

inline auto matrix::DecomposeToScale(const Mat4f& mx) -> Vec3f
{
  return Vec3f(mx[0], mx[5], mx[10]);
}

// Returns matrix contains basis for left handed system (right, up, fwd) 

inline Mat4f matrix::MakeLHBasis()
{
  Mat4f m {};
  m.SetCol(0, {1.f, 0.f, 0.f});
  m.SetCol(1, {0.f, 1.f, 0.f});
  m.SetCol(2, {0.f, 0.f, 1.f});
  m.SetCol(3, {0.f, 0.f, 0.f});
  return m;
}

// Returns matrix contains basis for right handed system (right, up, fwd) 

inline Mat4f matrix::MakeRHBasis()
{
  Mat4f m {};
  m.SetCol(0, {1.f, 0.f, 0.f});
  m.SetCol(1, {0.f, 1.f, 0.f});
  m.SetCol(2, {0.f, 0.f, -1.f});
  m.SetCol(3, {0.f, 0.f, 0.f});
  return m;
}

// Swap right and fwd axis to be convient with projection matrix
//  as it expect RUF

inline Mat4f matrix::SwapForwardRight(Mat4f mx)
{
  Vec3f right = mx.GetCol(0);
  mx.SetCol(0, mx.GetCol(2));
  mx.SetCol(2, right);
  return mx;
}

} // namespace gdm

#endif  // AH_GDM_MATRIX_H

// Notes:
//  https://bit.ly/2WpcyWV
