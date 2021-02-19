// *************************************************************
// File:    dx_colors.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include <math/vector4.h>

#ifndef RF_COLORS_LIST_H
#define RF_COLORS_LIST_H

namespace gdm::color {

  constexpr Vec4f White {1.f, 1.f, 1.f, 1.f};
  constexpr Vec4f LightGray {0.7f, 0.7f, 0.7f, 1.f};
  constexpr Vec4f Brown {0.647058845f, 0.164705887f, 0.164705887f, 1.0f};
  constexpr Vec4f LightYellow {1.0f, 1.0f, 0.878431439f, 1.0f };
  constexpr Vec4f Yellow {1.0f, 1.0f, 0.f, 1.0f };
  constexpr Vec4f Red {1.0f, 0.f, 0.f, 1.0f };
  constexpr Vec4f LightRed {0.7f, 0.f, 0.f, 1.0f };
  constexpr Vec4f Green {0.f, 1.f, 0.f, 1.0f };
  constexpr Vec4f LightGreen {0.f, 0.7f, 0.f, 1.0f };
  constexpr Vec4f Blue {0.f, 0.f, 1.f, 1.0f };
  constexpr Vec4f LightBlue {0.f, 0.f, 0.7f, 1.0f };
  constexpr Vec4f BlackBlue {0.f, 0.f, 0.2f, 1.0f };
  constexpr Vec4f Black {0.f, 0.f, 0.f, 1.0f };

} // namespace gdm::color

#endif  // RF_COLORS_LIST_H
