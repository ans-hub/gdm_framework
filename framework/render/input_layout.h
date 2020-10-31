// *************************************************************
// File:    input_layout.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_INPUT_LAYOUT_H
#define AH_INPUT_LAYOUT_H

#include <vector>

#include "defines.h"

namespace gdm {

struct InputLayout
{
  struct Element
  {
    const char* name_;
    uint size_;
    uint offset_;
    gfx::EFormatType type_;  
  };

public:
  InputLayout() = default;
  InputLayout(const std::initializer_list<Element>& list, gfx::EPrimitiveType type);

  auto GetData() const -> const std::vector<Element>& { return elements_; }
  auto GetSize() const -> uint { return size_; }
  auto GetPrimitiveType() const -> gfx::EPrimitiveType { return type_; }

private:
  std::vector<Element> elements_;
  uint size_ = 0;
  gfx::EPrimitiveType type_;

}; //struct InputLayout

namespace helpers {

  template<class T>
  auto MakeLayoutElement(const char* name) -> InputLayout::Element;
  template<class T>
  auto GetLayoutElementFormatType() -> gfx::EFormatType;

} // namespace helpers

} // namespace gdm

#include "input_layout.inl"

#endif // AH_INPUT_LAYOUT_H
