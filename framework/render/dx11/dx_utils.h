// *************************************************************
// File:    dx_utils.h
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#ifndef AH_DX_UTILS_H
#define AH_DX_UTILS_H

#include <Windows.h>
#include <d3d11.h>

#include <system/assert_utils.h>

namespace gdm {

namespace d3d_utils
{
  bool IsWindowFullscreen(HWND win_hndl);
  template<class T>
  void Release(T* ptr) { if (ptr) ptr->Release(); ptr = nullptr; }
  auto QueryRefreshRate(unsigned int scr_width, unsigned int scr_height, bool v_sync) -> DXGI_RATIONAL;
}

} // namespace gdm

#endif // AH_DX_UTILS_H