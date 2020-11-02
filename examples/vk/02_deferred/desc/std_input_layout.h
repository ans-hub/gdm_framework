// *************************************************************
// File:    desc/input_layout.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_STD_LAYOUT_DESC_H
#define SH_STD_LAYOUT_DESC_H

#include <render/defines.h>
#include <render/input_layout.h>

namespace gdm {

struct StdInputLayout : public InputLayout
{
  StdInputLayout() : InputLayout({
    helpers::MakeLayoutElement<Vec3f>("COORD"),
    helpers::MakeLayoutElement<Vec2f>("TEXUV"),
    helpers::MakeLayoutElement<Vec3f>("NORMAL"),
    helpers::MakeLayoutElement<Vec3f>("TANGENT")},
    gfx::EPrimitiveType::TRIANGLE_LIST)
  { }
};

} // namespace gdm

#endif // SH_STD_LAYOUT_DESC_H
