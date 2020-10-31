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
  AbstractImage() = default;
  AbstractImage(int width, int height, int depth, float r, float g, float b, float a = 1.f);

  virtual auto GetWidth() const -> int;
  virtual auto GetHeight() const -> int;
  virtual auto GetDepth() const -> int;
  virtual auto GetRaw() const -> const std::vector<unsigned char>&;

protected:
  int width_;
  int height_;
  int depth_;
  std::vector<unsigned char> data_;

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
