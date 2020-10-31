// *************************************************************
// File:    dx_utils.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "dx_utils.h"

bool gdm::d3d_utils::IsWindowFullscreen(HWND win_hndl)
{
  MONITORINFO monitor_info = { 0 };
  monitor_info.cbSize = sizeof(MONITORINFO);
  GetMonitorInfo(MonitorFromWindow(win_hndl, MONITOR_DEFAULTTOPRIMARY), &monitor_info);

  RECT window_rect;
  GetWindowRect(win_hndl, &window_rect);

  return window_rect.left == monitor_info.rcMonitor.left
      && window_rect.right == monitor_info.rcMonitor.right
      && window_rect.top == monitor_info.rcMonitor.top
      && window_rect.bottom == monitor_info.rcMonitor.bottom;
}

// This function was inspired by http://www.rastertek.com/dx11tut03.html

DXGI_RATIONAL gdm::d3d_utils::QueryRefreshRate(unsigned int screenWidth, unsigned int screenHeight, bool vsync)
{
  DXGI_RATIONAL refreshRate = { 0, 1 };
  if (vsync)
  {
    IDXGIFactory* factory;
    IDXGIAdapter* adapter;
    IDXGIOutput* adapterOutput;
    DXGI_MODE_DESC* displayModeList;

    HRESULT hr = CreateDXGIFactory( __uuidof(IDXGIFactory), (void**)&factory );
    assert(hr == S_OK && "Could not create DXGIFactory instance");

    hr = factory->EnumAdapters(0, &adapter);
    assert(hr == S_OK && "Failed to enumerate adapters");

    hr = adapter->EnumOutputs(0, &adapterOutput);
    assert(hr == S_OK && "Failed to enumerate adapter outputs");

    UINT numDisplayModes;
    hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr );
    assert(hr == S_OK && "Failed to query display mode list");

    displayModeList = new DXGI_MODE_DESC[numDisplayModes];
    assert(displayModeList);

    hr = adapterOutput->GetDisplayModeList( DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList );
    assert(hr == S_OK && "Failed to query display mode list");

    // Now store the refresh rate of the monitor that matches the width and height of the requested screen.
    for (UINT i = 0; i < numDisplayModes; ++i)
      if ( displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight )
        refreshRate = displayModeList[i].RefreshRate;

    delete [] displayModeList;
    d3d_utils::Release(adapterOutput);
    d3d_utils::Release(adapter);
    d3d_utils::Release(factory);
  }
  return refreshRate;
}
