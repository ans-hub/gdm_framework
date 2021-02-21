// *************************************************************
// File:    scissor_desc.h
// Author:  Novoselov Anton @ 2020
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_SCISSOR_DESC_H
#define AH_SCISSOR_DESC_H

namespace gdm {

struct ScissorDesc
{
  int offset_x;
  int offset_y;
  uint extent_x;
  uint extent_y;

}; // struct ScissorDesc

} // namespace gdm

#endif // AH_SCISSOR_DESC_H
