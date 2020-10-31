// *************************************************************
// File:    desc/input_layout.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef SH_LAYOUT_DESC_H
#define SH_LAYOUT_DESC_H

#include <render/input_layout.h>

namespace gdm {

struct StdInputLayout : public InputLayout
{
  StdInputLayout() : InputLayout({
    helpers::MakeLayoutElement<Vec3f>("COORD"),
    helpers::MakeLayoutElement<Vec2f>("TEXUV"),
    helpers::MakeLayoutElement<Vec3f>("NORMAL"),
    helpers::MakeLayoutElement<Vec3f>("TANGENT")})
  { }
};

struct DbgInputLayout : public InputLayout
{
  DbgInputLayout() : InputLayout({
    helpers::MakeLayoutElement<Vec3f>("COORD"),
    helpers::MakeLayoutElement<Vec3f>("COLOR")})
  { }
};

} // namespace gdm

#endif // SH_LAYOUT_DESC_H
