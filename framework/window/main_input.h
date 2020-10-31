// *************************************************************
// File:    input.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_GDM_INPUT_H
#define AH_GDM_INPUT_H

#if defined (_WIN32)
#include "window/win/dx_input.h"
namespace gdm
{
  using MainInput = DxInput;
}
#else
#include "window/linux/main_window.h"
namespace gdm
{
  using MainInput = LinuxInput;
}
#endif

#endif // AH_GDM_INPUT_H