// *************************************************************
// File:    viewport_desc.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_VIEWPORT_DESC_H
#define AH_VIEWPORT_DESC_H

namespace gdm {

struct ViewportDesc
{
  float x_;
  float y_;
  float width_;
  float height_;
  float min_depth_;
  float max_depth_;

}; // struct ViewportDesc

} // namespace gdm

#endif // AH_VIEWPORT_DESC_H
