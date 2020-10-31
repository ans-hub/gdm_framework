// *************************************************************
// File:    input_layout.inl
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "input_layout.h"

#include <initializer_list>

#include "math/vector2.h"
#include "math/vector3.h"
#include "math/vector4.h"

// --public

inline gdm::InputLayout::InputLayout(const std::initializer_list<Element>& list, gfx::EPrimitiveType type)
  : elements_{}
  , size_{}
  , type_{type}
{
  for(const auto& element : list)
  {
    elements_.push_back(element);
    elements_.back().offset_ = size_;
    uint element_size = elements_.back().size_;
    size_ += element_size;
  }
}

// --private

template<class T>
inline auto gdm::helpers::GetLayoutElementFormatType() -> gfx::EFormatType
{
  if constexpr (std::is_same_v<T, float>)
    return gfx::EFormatType::F1;
  else if constexpr (std::is_same_v<T, Vec2f>)
    return gfx::EFormatType::F2;
  else if constexpr (std::is_same_v<T, Vec3f>)
    return gfx::EFormatType::F3;
  else
  {
    ASSERTF(false, "Unknown type format")
  }
}

// --helpers

template<class T>
inline auto gdm::helpers::MakeLayoutElement(const char* name) -> gdm::InputLayout::Element
{
  InputLayout::Element elem {name, sizeof(T), 0, GetLayoutElementFormatType<T>()};
  return elem;
}
