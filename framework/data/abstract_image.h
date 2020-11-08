// *************************************************************
// File:    abstract_image.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_ABSTRACT_IMAGE_H
#define AH_GDM_ABSTRACT_IMAGE_H

#include <stdexcept>
#include <iostream>
#include <vector>

namespace gdm {

struct AbstractImage
{
  using DataType = unsigned char;
  using StorageType = std::vector<DataType>;

  AbstractImage() = default;
  AbstractImage(int width, int height, int depth, float r, float g, float b, float a = 1.f);

  virtual auto GetWidth() const -> unsigned;
  virtual auto GetHeight() const -> unsigned;
  virtual auto GetDepth() const -> unsigned;
  virtual auto GetRaw() const -> const StorageType&;

protected:
  int width_;
  int height_;
  int depth_;
  StorageType data_;

}; // struct AbstractImage

struct ImgExcept : std::runtime_error
{
  ImgExcept(const char* msg) : std::runtime_error(msg) { }

}; // struct ImgExcept

namespace helpers {

  template <class T>
  inline void ReadFromStream(std::istream& stream, T& t);
  
} // namespace helpers

} // namespace gdm

#include "abstract_image.inl"

#endif // AH_GDM_ABSTRACT_IMAGE_H
